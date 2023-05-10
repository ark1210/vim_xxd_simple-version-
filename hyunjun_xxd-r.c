#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define BUFFER_SIZE 1024

void convert_to_hex(FILE *input, FILE *output) {
    int c;
    size_t byte_count = 0;
    char ascii[17] = {0};
    while ((c = fgetc(input)) != EOF) {
        if (byte_count % 16 == 0) {
            fprintf(output, "%08zX: ", byte_count);
        }
        fprintf(output, "%02X", c);

        if (byte_count % 2 == 1) {
            fprintf(output, " ");
        }

        ascii[byte_count % 16] = (c >= 32 && c <= 126) ? c : '.';

        byte_count++;

        if (byte_count % 16 == 0) {
            fprintf(output, "%s\n", ascii);
            memset(ascii, 0, sizeof(ascii));
        }
    }

    if (byte_count % 16 != 0) {
        for (int i = byte_count % 16; i < 16; i++) {
            if (i % 2 == 0) {
                fprintf(output, "  ");
            } else {
                fprintf(output, "   ");
            }
        }
        ascii[byte_count % 16] = '\0';
        fprintf(output, "%s\n", ascii);
    }
}

void convert_to_ascii(FILE *input, FILE *output) {
    size_t count = 0;
    unsigned int byte;
    char buf[17];
    int bytesRead;
    while (fscanf(input, "%08zX: ", &count) == 1) {
        bytesRead = 0;
        for (int i = 0; i < 16; i++) {
            if (fscanf(input, "%02X", &byte) == 1) {
                buf[i] = (char)byte;
                bytesRead++;
            }
            if (i % 2 == 1) {
                fscanf(input, " ");
            }
        }
        fwrite(buf, 1, bytesRead, output); // Only write bytesRead bytes
        fscanf(input, "%*[^\n]"); // skip the rest of the line
        fscanf(input, "\n"); // skip the newline character
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3 && argc != 4) {
        fprintf(stderr, "Usage: %s [-r] <input> <output>\n", argv[0]);
        return 1;
    }

    bool reverse = false;
    if(argc == 4) {
        if(strcmp(argv[1], "-r") == 0) {
            reverse = true;
        } else {
            fprintf(stderr, "Usage: %s [-r] <input> <output>\n", argv[0]);
            return 1;
        }
    }

    const char *input_filename = reverse ? argv[2] : argv[1];
    const char *output_filename = reverse ? argv[3] : argv[2];

    FILE *input;
    if (strcmp(input_filename, "-") == 0) {
        // Use stdin as input
        input = stdin;
    } else {
        // Create temporary input file
        input = tmpfile();
        if (!input) {
            perror("Error creating temporary input file");
            return 1;
        }
        // Copy input file to temporary input file
        FILE *original_input = fopen(input_filename, "r");
        if (!original_input) {
            perror("Error opening input file");
            fclose(input);
            return 1;
        }
        char buffer[BUFFER_SIZE];
        size_t n;
        while ((n = fread(buffer, 1, sizeof(buffer), original_input)) > 0) {
            fwrite(buffer, 1, n, input);
        }
        fclose(original_input);
        // Reset file position indicator to the beginning of the file
        rewind(input);
    }

    FILE *output;
    if (strcmp(output_filename, "-") == 0) {
        // Use stdout as output
        output = stdout;
    } else {
        // Create output file
        output = fopen(output_filename, "w");
        if (!output) {
            perror("Error opening output file");
            fclose(input);
            return 1;
        }
    }

    if (reverse) {
        convert_to_ascii(input, output);
    } else {
        convert_to_hex(input, output);
    }

    fclose(input);
    fclose(output);

    return 0;
}
