// 14/08/2022 2239hrs - 19/08/2022 1911hrs implementation time
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define SIZECHAR 8

int shiftkeys[9] = {0, 1, 3, 7, 15, 31, 63, 127, 255};
unsigned char MAINBYTE = 0, CACHEBYTE = 0;
int mainbyte_size=0, cachebyte_size=0, BYTESIZE=8;
unsigned char ISBYTE = 0;

unsigned char BYTESTACK[32];
int nbytestack = 0;

unsigned char getbitat(unsigned char value, unsigned char n){
	return (value >> n) & 1;
}

int compress_byte_or(unsigned char value){
    // attempt to compress a single byte using OR logic
    unsigned char head = 0x0, body = 0x0;
    int icount = 0, out = 0;
    unsigned char c0, c1, or_;
    for (int i = SIZECHAR-1; i >= 0; i-=2){
        c0 = getbitat(value, i);
        c1 = getbitat(value, i-1);
        or_ = c0 | c1;
        head = (head << 1) | or_;
        if(or_ == 1){
            if (c0 == 0 && c1 == 1){ // append 0
                body <<= 1; // push in 0
                icount+=1;
            }else if(c0 == 1 && c1 == 0){ // append 10
                body = (body << 2) | 2;
                icount+=2;
            }else if (c0 == 1 && c1 == 1){
                body = (body << 2) | 3;
                icount+=2;
            }
        }
    }
    // add compressed as len bits to right (len <= 12)
    out = head;
    out = (out << icount) | (body & shiftkeys[icount]);
    // add length as 4 bits;
    out = (out << 4) | (4 + icount);
    return out;
}

int compress_byte_and(unsigned char value){
    // attempt to compress a single byte using OR logic
    unsigned char head = 0x0, body = 0x0;
    int icount = 0, out = 0;
    unsigned char c0, c1, and_;
    for (int i = SIZECHAR-1; i >= 0; i-=2){
        c0 = getbitat(value, i);
        c1 = getbitat(value, i-1);
        and_ = c0 & c1;
        head = (head << 1) | and_;
        if(and_ == 0){
            if (c0 == 0 && c1 == 0){ // append 0
                body <<= 1; // push in 0
                icount+=1;
            }else if(c0 == 0 && c1 == 1){ // append 10
                body = (body << 2) | 2;
                icount+=2;
            }else if (c0 == 1 && c1 == 0){
                body = (body << 2) | 3;
                icount+=2;
            }
        }
    }
    // add compressed as len bits to right (len <= 12)
    out = head;
    out = (out << icount) | (body & shiftkeys[icount]);
    // add length as 4 bits;
    out = (out << 4) | (4 + icount);
    return out;
}

int argmax(unsigned char *arr, int indx_start, int span){
    // returns the absolute index of the maximum value
    int mx=arr[indx_start], maxdx = indx_start;
    for (int i=indx_start; i < indx_start + span; i++){
        if (arr[i] > mx){
            mx = arr[i];
            maxdx = i;
        }
    }
    return maxdx;
}

unsigned int sum(unsigned char *arr, int indx_start, int span){
    // find sum of slice
    unsigned int total = 0;
    for (int i=indx_start; i < indx_start + span; i++){
        total += arr[i];
    }
    return total;
}

unsigned char get_nzeros(int val, int n){
    // get number of zeros in up to n
    unsigned char nzeros = 0;
    for (int i=0; i<n; i++){
        nzeros += (getbitat(val, i) == 0);
    }
    return nzeros;
}

unsigned char CONVSTATE = 1; // a boolean state of whether values were shifted
int get_argmax_and_state_zeros(unsigned char *arr, unsigned char *zarr, int indx_start, int span){
    // returns argmax and whether to convert or not based on total number of zeros
    unsigned nzerosA = 0, nzerosB = 0; // total of raw arr & shifted array
    CONVSTATE = 1;
    int mx=arr[indx_start], maxdx = indx_start;
    for (int i=indx_start; i < indx_start + span; i++){
        nzerosA += zarr[ arr[i] ];
        if (arr[i] > mx){
            mx = arr[i];
            maxdx = i;
        }
    }

    unsigned char tmp;
    for (int i=indx_start; i<indx_start+span; i++){
        if (i == maxdx){
            tmp = arr[maxdx];
        }else{
            tmp = arr[maxdx] - arr[i];
            if ((i < maxdx) && (tmp == arr[maxdx])){ // there is a 0 before argmax
                CONVSTATE = 0;
            }
        }
        nzerosB += zarr[tmp];
    }
    CONVSTATE = (CONVSTATE == 0) ? CONVSTATE : nzerosA < nzerosB;
    return maxdx;
}

int get_argmax_and_state_total(unsigned char *arr, int indx_start, int span){
    // returns argmax and whether to convert or not based on total of arr's span
    unsigned TOTALA = 0, TOTALB = 0; // total of raw arr & shifted array
    CONVSTATE = 1;
    int mx=arr[indx_start], maxdx = indx_start;
    for (int i=indx_start; i < indx_start + span; i++){
        TOTALA += arr[i];
        if (arr[i] > mx){
            mx = arr[i];
            maxdx = i;
        }
    }

    unsigned char tmp;
    for (int i=indx_start; i<indx_start+span; i++){
        if (i == maxdx){
            tmp = arr[maxdx];
        }else{
            tmp = arr[maxdx] - arr[i];
            if ((i < maxdx) && (tmp == arr[maxdx])){ //there is a 0 before argmax
                CONVSTATE = 0;
            }
        }
        TOTALB += tmp;
    }
    CONVSTATE = (CONVSTATE == 0) ? CONVSTATE : TOTALB < TOTALA;
    return maxdx;
}

void makebyte(unsigned char value, int n){
	// makes a byte from continuous concatenation of bit-sequences
	if (ISBYTE){
        MAINBYTE = CACHEBYTE;
        mainbyte_size = cachebyte_size;
        CACHEBYTE = 0;
        cachebyte_size = 0;
        ISBYTE = 0;
	}

    if (BYTESIZE-mainbyte_size >= n){
        MAINBYTE = (MAINBYTE << n) | (value & shiftkeys[n]);
        mainbyte_size += n;
    }else{
        int rem = BYTESIZE-mainbyte_size;
        MAINBYTE = (MAINBYTE << rem) | (value >> (n - rem));
        mainbyte_size += rem;
        CACHEBYTE = value & shiftkeys[n - rem];
        cachebyte_size = n - rem;
    }
    if (mainbyte_size == BYTESIZE){ ISBYTE=1; }
}

void makebytes(int val, int n){
    // make bytes from longer sequence and places results on stack
    if (n > BYTESIZE){
        unsigned char tmp = (val >> (n-BYTESIZE)) & shiftkeys[BYTESIZE];
        makebyte(tmp, BYTESIZE);
        if (ISBYTE){
            BYTESTACK[nbytestack++] = MAINBYTE;
        }
        makebyte(val & shiftkeys[n-BYTESIZE], n-BYTESIZE);
        if (ISBYTE){
            BYTESTACK[nbytestack++] = MAINBYTE;
        }
    }else{
        makebyte(val, n);
        if (ISBYTE){
            BYTESTACK[nbytestack++] = MAINBYTE;
        }
    }
}

int main(){
    unsigned char NZEROS[256];
    for (int i=0; i<256; i++){
        NZEROS[i] = get_nzeros(i, 8);
    }

    unsigned char chunk_size = 7;
    int BUFFER_SIZE = 1024*1024;
    int INTM_SIZE = 1024;
    unsigned char *MAINBUFFER = (unsigned char *) malloc(BUFFER_SIZE * sizeof(char));
    unsigned char *INTMBUFFER = (unsigned char *) malloc(INTM_SIZE * sizeof(char));

    if (MAINBUFFER == NULL){
        printf("\nAllocation request failed!");
        exit(1);
    }

    printf("---OR COMPRESSOR--- by Joseph M\n");
    printf("Enter FILEPATH : ");
    char filepath[128];
    scanf("%[^\n]%*c", filepath);

    char extfound = 0;
    char filename[strlen(filepath)]; int fncount = 0;
    char file_ext[25]; int extcount = 0;
    for (int i=strlen(filepath)-1; i >=0; i--){
        if (filepath[i] == '.'){ extfound = 1; continue; }
        if ((filepath[i] == '\\') || (filepath[i] == '/')){ break; } // stop on filename only
        if (extfound == 0){
            file_ext[extcount++] = filepath[i]; // reversed
        }else{
            filename[fncount++] =  filepath[i]; // reversed
        }
    }
    filename[fncount] = '\0';
    file_ext[extcount] = '\0';
    strrev(filename);
    strrev(file_ext);

    FILE *fr = fopen(filepath, "rb");
	if (fr == NULL){
		printf("\nError: No such File.");
		exit(1);
	}
	int cnt = (fncount == 0) ? extcount : fncount;
	char outname[cnt + 5];
	strcpy(outname, (fncount == 0) ? file_ext : filename);
	char c_extension[5] = ".cmp";
    strcat(outname, c_extension);

	FILE *fw = fopen(outname, "wb");
	if (fw == NULL){
		printf("\nError: Failed to create file.");
		exit(1);
	}

    fwrite((char *) &chunk_size, sizeof(char), 1, fw);
	int hasfname = fncount != 0;
	if (hasfname){
        fwrite((char *) &fncount, sizeof(char), 1, fw);
        fwrite(filename, sizeof(char), fncount, fw);
        fwrite((char *) &extcount, sizeof(char), 1, fw);
        fwrite(file_ext, sizeof(char), extcount, fw);
	}else{
	    fwrite((char *) &extcount, sizeof(char), 1, fw);
        fwrite(file_ext, sizeof(char), extcount, fw);
        fwrite((char *) &fncount, sizeof(char), 1, fw);
	}

	printf("\nSuccessfully opened files.");
	printf("\nPlease wait...");
	int readbytes = 0, intmsize=0, buff_level;
	unsigned char tmp;
	int itr=0, i, compressed, bspan, target;
	int mxindx;
	readbytes = fread(MAINBUFFER, sizeof(char), BUFFER_SIZE, fr);
	buff_level = readbytes;

	while(buff_level > 0){
        //printf("\nbuff_level = %d", buff_level);
        if (buff_level < chunk_size){ // append_fill
            memcpy(MAINBUFFER, MAINBUFFER+itr, buff_level);
            readbytes = fread(MAINBUFFER + buff_level, sizeof(char), BUFFER_SIZE-buff_level, fr);
            buff_level += readbytes;
            itr=0;
        }

        // DO COMPRESSION
        bspan = (buff_level > chunk_size) ? chunk_size : buff_level ;
        // use state engine : zeros or total
        mxindx = get_argmax_and_state_zeros(MAINBUFFER, NZEROS, itr, bspan);
        makebytes(CONVSTATE, 1);
        target = itr + bspan;
        for ( ; itr<target; itr++){
            if (CONVSTATE){
                tmp = MAINBUFFER[mxindx] - MAINBUFFER[itr];
                if (itr == mxindx){
                    tmp = MAINBUFFER[itr];
                }
            }else{
                tmp = MAINBUFFER[itr];
            }
            // use logical method : AND or OR
            compressed = compress_byte_or(tmp);
            makebytes(compressed >> 4, compressed & shiftkeys[4]);
        }
        buff_level -= bspan;

        for(i=0; i<nbytestack; i++){
            INTMBUFFER[intmsize++] = BYTESTACK[i];
            if (intmsize >= INTM_SIZE){
                fwrite((char *)INTMBUFFER, sizeof(char), intmsize, fw);
                intmsize = 0;
            }
        }
        nbytestack = 0;
	}
	// write remainder INTMBUFFER
	if(intmsize > 0){
        fwrite((char *)INTMBUFFER, sizeof(char), intmsize, fw);
        intmsize = 0;
	}
	// write remainder BYTE;
	unsigned char lastbyte;
	if(cachebyte_size > 0){
        lastbyte = (CACHEBYTE << (BYTESIZE - cachebyte_size)) | shiftkeys[BYTESIZE - cachebyte_size];
        fwrite((char *) &lastbyte, sizeof(char), 1, fw);
	}else if(mainbyte_size > 0){
        lastbyte = (MAINBYTE << (BYTESIZE - mainbyte_size)) | shiftkeys[BYTESIZE - mainbyte_size];
        fwrite((char *) &lastbyte, sizeof(char), 1, fw);
	}

	free(MAINBUFFER);
	free(INTMBUFFER);
    fclose(fr);
    fclose(fw);
    printf("\nDone!!!");
}
