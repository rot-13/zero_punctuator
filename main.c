#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <tgmath.h>
#include <time.h>

#pragma pack(1)

typedef struct {
    char chunkId[4];
    uint32_t chunkSize;
    char format[4];
    char fmt[4];
    uint32_t fmtSize;
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
    char data[4];
    uint32_t dataSize;
} wav_header;

typedef struct {
    uint32_t start;
    uint32_t end;
} range;

#pragma pack(0)


#define TIME_SLICE_MS 10
#define VOLUME_THRESHOLD 300.0
#define SILENCE_MIN_TIME 100

/*char* readString(FILE *file, size_t length) {*/
    /*char *str = (char*)malloc(length);*/
    /*fread(str, 1, length, file);*/
    /*return str;*/
/*}*/

int main() {
    FILE *file = fopen("audio.wav", "r");
    wav_header header;
    fread(&header, sizeof(wav_header), 1, file);
    printf("%.4s\n", header.chunkId);
    printf("%d\n", header.chunkSize);
    printf("%.4s\n", header.format);
    printf("%.4s\n", header.fmt);
    printf("%d\n", header.fmtSize);
    printf("%d\n", header.audioFormat);
    printf("%d\n", header.numChannels);
    printf("%d\n", header.sampleRate);
    printf("%d\n", header.byteRate);
    printf("%d\n", header.blockAlign);
    printf("%d\n", header.bitsPerSample);
    printf("%.4s\n", header.data);
    printf("%d\n", header.dataSize);

    struct timespec sleepTime;
    struct timespec timeSlept;

    sleepTime.tv_sec = 1;
    sleepTime.tv_nsec = 0;
    printf("3\n");
    nanosleep(&sleepTime, &timeSlept);
    printf("2\n");
    nanosleep(&sleepTime, &timeSlept);
    printf("1\n");
    nanosleep(&sleepTime, &timeSlept);

    int16_t sampleLeft;
    int16_t sampleRight;
    uint32_t amountRead = 0;
    sleepTime.tv_sec = 0;
    sleepTime.tv_nsec = 10000000;
    uint32_t silenceStart = 0;
    uint32_t silenceEnd = 0;
    uint32_t currentTime = 0;
    uint8_t inSilence = 0;
    uint32_t iterations = header.sampleRate / (1000/TIME_SLICE_MS);

    FILE *rangeFile = fopen("ranges.rng", "w");

    while (amountRead < header.dataSize) {
        uint32_t i;
        double sum = 0;
        for (i = 0; i < iterations && amountRead < header.dataSize; ++i) {
            // 10ms chunks
            amountRead += fread(&sampleLeft, sizeof(sampleLeft), 1, file) * sizeof(sampleLeft);
            amountRead += fread(&sampleRight, sizeof(sampleRight), 1, file) * sizeof(sampleRight);
            sum += sampleLeft * sampleLeft;
        }
        double amp = sqrt(sum / i);

        if (inSilence) {
            if (amp > VOLUME_THRESHOLD) {
                silenceEnd = currentTime;
                inSilence = 0;
                if (silenceEnd - silenceStart > SILENCE_MIN_TIME) {
                    range rng;
                    rng.start = silenceStart;
                    rng.end = silenceEnd;
                    fwrite(&rng, sizeof(rng), 1, rangeFile);
                    printf("\n\x1b[30;47mFound silent range: %dms - %dms. TOTAL: %.2f seconds\n", rng.start, rng.end, (rng.end - rng.start)/1000.0f);
                    sleepTime.tv_sec = 1;
                    nanosleep(&sleepTime, &timeSlept);
                    sleepTime.tv_sec = 0;
                }
                printf("\x1b[0m");
            }
        } else {
            if (amp < VOLUME_THRESHOLD) {
                silenceStart = currentTime;
                inSilence = 1;
                printf("\x1b[41m");
            }
        }

        currentTime += TIME_SLICE_MS;
        printf("\n");
        int numChars = (int)(amp/4000.0*80);
        for (i = 0; i < 80; ++i) {
            if (i < numChars) {
                printf("#");
            } else {
                printf(" ");
            }
        }
        nanosleep(&sleepTime, &timeSlept);
    }

    printf("\ndone\n");

    fclose(file);
    fclose(rangeFile);

    return 0;
}
