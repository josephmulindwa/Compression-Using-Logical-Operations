// 15/08/2022 1840hrs -  19/08/2022 1907hrs implementation time
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define BYTESIZE 8

unsigned char getbitat(unsigned char value, unsigned char n){
	return (value >> n) & 1;
}

unsigned char decompress_byte_with_length(int value){
    // returns the original byte from the value; if value includes length
    unsigned char shiftkeys[5] = {0, 1, 3, 7, 15};
    int len = value & 0xf; // 15
    value >>= 4;
    unsigned char out[8] = {0, 0, 0, 0, 0, 0, 0, 0};

    int shifter = 15;
    // access head
    shifter <<= (len - 4);
    int head = (value & shifter) >> (len - 4);
    unsigned char hc, ic, tc;
    int i, j, icount = 0, tcount = 0;
    for (i=3; i>=0; i--){
        hc = getbitat(head, i);
        if (hc == 1){
            icount++;
        }else{
            out[i*2] = 0;
            out[i*2+1] = 0;
        }
    }

    shifter = shiftkeys[icount];
    int rlen = len - 4 - icount;
    shifter <<= rlen;
    int intm = (value & shifter) >> rlen;
    for (i=3; i>=0; i--){
        hc = getbitat(head, i);
        if (hc == 1){
            ic = getbitat(intm, --icount);
            if (ic == 0){
                out[i*2+1] = 0;
                out[i*2] = 1;
            }else{
                tc = getbitat(value, --rlen);
                if (tc == 0){
                    out[i*2+1] = 1;
                    out[i*2] = 0;
                }else{
                    out[i*2+1] = 1;
                    out[i*2] = 1;
                }
            }
        }
    }
    unsigned char bout = 0x0;
    for (i=BYTESIZE-1; i>=0; i--){
        bout = (bout << 1) | out[i];
    }
    return bout;
}

int argmax(unsigned char *arr, int indx_start, int span){
    // returns the absolute argmax of the range
    int mx=arr[indx_start], maxdx = indx_start;
    for (int i=indx_start; i < indx_start + span; i++){
        if (arr[i] > mx){
            mx = arr[i];
            maxdx = i;
        }
    }
    return maxdx;
}

int shiftkeys[9] = {0, 1, 3, 7, 15, 31, 63, 127, 255};

int main(){
    unsigned char chunk_size;
    int BUFFER_SIZE = 1024*1024;
    //int INTM_SIZE = 1024;
    unsigned char CHUNK[chunk_size];
    unsigned char TMP[chunk_size];
    int chunk_index = 0;
    unsigned char *MAINBUFFER = (unsigned char *) malloc(BUFFER_SIZE * sizeof(char));
    //unsigned char *INTMBUFFER = (unsigned char *) malloc(INTM_SIZE * sizeof(char));

    if (MAINBUFFER == NULL){
        printf("\nAllocation request failed!");
        exit(1);
    }

    printf("---OR DECOMPRESSOR--- by Joseph M\n");
    printf("Enter FILEPATH : ");
    char filepath[128];
    scanf("%[^\n]%*c", filepath);

    FILE *fr = fopen(filepath, "rb");
	if (fr == NULL){
		printf("\nError: No such File.");
		exit(1);
	}

	fread((char *) &chunk_size, sizeof(char), 1, fr);
	unsigned char fncount;
	fread((char *) &fncount, sizeof(char), 1, fr);
	char filename[fncount + 1];
	fread(filename, sizeof(char), fncount, fr);
	filename[fncount] = '\0';

	unsigned char extcount;
	fread((char *) &extcount, sizeof(char), 1, fr);
	char file_ext[extcount + 1];
	fread(file_ext, sizeof(char), extcount, fr);
	file_ext[extcount] = '\0';

	char point[2] =  ".";
	char outname[fncount + extcount + 2];
	strcpy(outname, filename);
    strcat(outname, point);
    strcat(outname, file_ext);

	FILE *fw = fopen(outname, "wb");
    if (fw == NULL){
		printf("\nError: Failed to create file.");
		exit(1);
	}

	printf("\nSuccessfully opened files.");
	printf("\nPlease wait...");
	int readbytes = 0, intmsize=0;
	unsigned char CONVSTATE;
	unsigned char head, tmp, curr, tick;
	int bit_index = BYTESIZE;
	int itr, formed;
	while((readbytes = fread(MAINBUFFER, sizeof(char), BUFFER_SIZE, fr)) > 0){
        unsigned char out[BYTESIZE] = {0, 0, 0, 0, 0, 0, 0, 0};
        itr = 0;
        while (itr < readbytes){
            curr = MAINBUFFER[itr];
            formed = 0;
            if (chunk_index == 0){
                if (bit_index == 0){
                    if(itr+1 >= readbytes){
                        readbytes = fread(MAINBUFFER, sizeof(char), BUFFER_SIZE, fr);
                        if (readbytes == 0){ break; }
                        itr = -1;
                    }
                    curr = MAINBUFFER[++itr];
                    CONVSTATE = (curr >> (BYTESIZE - 1)) & shiftkeys[1];
                    bit_index = BYTESIZE - 1;
                }else{
                    CONVSTATE = (curr >> (bit_index-1)) & shiftkeys[1];
                    bit_index -= 1;
                }
            }

            if (bit_index < 4){
                head = curr & shiftkeys[bit_index];
                if (itr+1 >= readbytes){
                    readbytes = fread(MAINBUFFER, sizeof(char), BUFFER_SIZE, fr);
                    if (readbytes == 0){ break; }
                    itr = -1;
                }
                curr = MAINBUFFER[++itr];
                head = (head << (4-bit_index)) | (curr >> (BYTESIZE - (4-bit_index)));
                bit_index = BYTESIZE - (4-bit_index);
            }else{
                head = (curr >> (bit_index - 4)) & shiftkeys[4];
                bit_index -= 4;
            }

            for (int i=3; i>=0; i--){
                if (getbitat(head, i) == 0){
                    out[i*2+1] = 0;
                    out[i*2] = 0;
                    formed += 2;
                }else{
                    tmp = 0;
                    tick = 1;
                    while (tick){
                        if (bit_index < 1){
                            if (itr+1 >= readbytes){
                                readbytes = fread(MAINBUFFER, sizeof(char), BUFFER_SIZE, fr);
                                if (readbytes == 0){ break; }
                                itr = -1;
                            }
                            curr = MAINBUFFER[++itr]; // ensure operation possible
                            tmp = (tmp << 1) | (curr >> (BYTESIZE - 1));
                            bit_index = BYTESIZE - 1;
                        }else{
                            tmp = (tmp << 1) | ((curr >> (bit_index - 1)) & shiftkeys[1]);
                            bit_index -= 1;
                        }
                        if (tmp==0 || tmp==2 || tmp==3){
                            tick = 0;
                        }
                    }

                    if (tmp == 0){
                        out[i*2+1] = 0;
                        out[i*2] = 1;
                        formed += 2;
                    }else if(tmp == 2){
                        out[i*2+1] = 1;
                        out[i*2] = 0;
                        formed += 2;
                    }else if(tmp == 3){
                        out[i*2+1] = 1;
                        out[i*2] = 1;
                        formed += 2;
                    }
                }
            }

            if (formed){
                unsigned char bout = 0x0;
                for (int i=7; i>=0; i--){
                    bout = (bout << 1) | out[i];
                }

                CHUNK[chunk_index++] = bout;

                if (chunk_index >= chunk_size){
                    int amax = argmax(CHUNK, 0, chunk_size);
                    for (int j=0; j<chunk_index; j++){
                        if(!CONVSTATE || (j == amax)){
                            TMP[j] = CHUNK[j];
                        }else{
                            TMP[j] = CHUNK[amax] - CHUNK[j];
                        }
                    }
                    fwrite((char *) TMP, sizeof(char), chunk_size, fw);
                    chunk_index = 0;
                }
            }
        }
	}

	if (chunk_index > 0){
        int amax = argmax(CHUNK, 0, chunk_index);
        for (int j=0; j<chunk_index; j++){
            if(!CONVSTATE || (j == amax)){
                TMP[j] = CHUNK[j];
            }else{
                TMP[j] = CHUNK[amax] - CHUNK[j];
            }
        }
        fwrite((char *) TMP, sizeof(char), chunk_index, fw);
        chunk_index = 0;
    }

	free(MAINBUFFER);
	//free(INTMBUFFER);
    fclose(fr);
    fclose(fw);
    printf("\nDone!!!");
}

