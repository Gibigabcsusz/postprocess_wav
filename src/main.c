#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "wav_types.h"
#include "wav_header.h"
#include "wav_data.h"
#include "wav_process.h"
#include "helper_functions.h"

#define CODE_LENGTH 32

int main(int argc, char *argv[]) {
  FILE *fp_in = NULL, *fp_out = NULL;
  char *input_file_name;
  char *output_file_name = NULL;
  union header_data *header;
  short* data = NULL;

  verify_machine();

  if (argc != 4) {
    printf("Usage: wave <bitrate> <input file> <output file>\n");
    return 1;
  }
  /* Process arguments */
  
  // bitrate
    double bitrate;
    sscanf(argv[1], "%lf", &bitrate);
    printf("bitrate: %lf\n", bitrate);
 
  // input filename
    input_file_name = argv[2];
    fp_in = fopen(input_file_name, "r");
    if(fp_in == NULL)
    {
        printf("error: could not open input file\n");
        return 1;
    }

  // output filename
    output_file_name = argv[3];
    fp_out = fopen(output_file_name, "w");
    if(fp_out == NULL)
    {
        fclose(fp_in);
        printf("error: could not open output file\n");
        return 1;
    }

    header = (union header_data *) malloc(sizeof(union header_data));
    read_header(fp_in, header, input_file_name);
    
    print_header_data(header);
    data = read_data(fp_in, header);

  // extract info from header
    
    int bits_per_sample = header->header.bits_per_sample.short_value;
    double sample_rate = (double)header->header.sample_rate.int_value;
    int samples_per_symbol = (int)(sample_rate/bitrate);
    int data_bytes = header->header.subchunk2_size.int_value;
    int num_samples = data_bytes/(bits_per_sample/8);
    int num_symbols = num_samples/samples_per_symbol;

    printf("\nBits per sample: %d\n",bits_per_sample);
    printf("Sample rate: %lf\n",sample_rate);
    printf("Samples per symbol: %d\n",samples_per_symbol);
    printf("Data length in bytes: %d\n",data_bytes);
    printf("Number of samples: %d\n",num_samples);
    printf("Last Sample: %d\n",data[num_samples-1]);
    printf("Number of symbols: %d\n",num_symbols);
    
    double original_code[CODE_LENGTH] = {1,1,1,1,1,1,0,1,0,1,1,0,1,1,0,1,0,0,0,1,1,1,0,0,1,1,1,0,1,0,0,0};
    //double original_code[CODE_LENGTH] = {1,1,1,0,0,0,1,0,0,1,0};
    double code[CODE_LENGTH]; // the original code in reverse

    for(int i=0; i<CODE_LENGTH; i++)
    {
        code[i] = original_code[CODE_LENGTH-1-i]*2-1;
        printf("%lf\n", code[i]);
    }

  // convolution
    int i, j, step=20;
    double accu = 0.0;
    for(i=0; i<num_samples-(CODE_LENGTH+1)*samples_per_symbol; i+=step)
    {
        //fprintf(fp_out, "%d\n", data[i]);
        accu = 0.0;
        for(j=0; j<CODE_LENGTH; j++)
            accu += data[i+j*samples_per_symbol]*code[j];
        fprintf(fp_out, "%lf\n", accu);
    }

    
    fclose(fp_in);
    fclose(fp_out);
    if(data) free(data);

    return 0;
}

/*
  printf("ChunkID: %.4s\n", file_bytes->header.chunk_id);
  printf("ChunkSize: %d\n", file_bytes->header.chunk_size.int_value);
  printf("Format: %.4s\n", file_bytes->header.format);
  printf("Subchunk1 ID: %.4s\n", file_bytes->header.subchunk1_id);
  printf("Subchunk1 Size: %d\n", file_bytes->header.subchunk1_size.int_value);
  printf("Audio Format: %d\n", file_bytes->header.audio_format.short_value);
  printf("Num Channels: %d\n", file_bytes->header.num_channels.short_value);
  printf("Sample Rate: %d\n", file_bytes->header.sample_rate.int_value);
  printf("ByteRate: %d\n", file_bytes->header.byte_rate.int_value);
  printf("Block Align: %d\n", file_bytes->header.block_align.short_value);
  printf("Bits per sample: %d\n", file_bytes->header.bits_per_sample.short_value);
  printf("Subchunk2 Id: %.4s\n", file_bytes->header.subchunk2_id);
  printf("Subchunk2 size: %d\n", file_bytes->header.subchunk2_size.int_value);
 * */



  /*
  // Process flags
  int i;
  for (i=1; i < argc ; i++) {
    if (*argv[i] == '-') {
      if (strlen(argv[i]) != 2) {
        fprintf(stderr, "Invalid option: %s\n", argv[i]);
        return 1;
      }
      switch (argv[i][1]) {
      case 'h':
        print_help();
        return 0;
      case 'r':
        flags |= REVERSE;
        flags |= WRITE;
        break;
      case 'v':
        flags |= CONTROL_VOLUME;
        flags |= WRITE;
        break;
      case 'o':
        flags |= WRITE;
        i++;
        if (i < argc && *argv[i] != '-')
          // Verify the file doesn't exit
          if (fopen(argv[i], "r") == NULL)
            output_file_name = argv[i];
          else {
            fprintf(stderr, "Invalid output file.\n"
                    "File %s already exists\n", argv[i]);
            return 1;
          }
        else {
          fprintf(stderr, "Specify a valid output file name after -o flag\n");
          return 1;
        }
        break;
      case 'p':
        flags |= PRINT;
        break;
      case 'L':
        flags |= MUTE_LEFT;
        flags |= WRITE;
        break;
      case 'R':
        flags |= MUTE_RIGHT;
        flags |= WRITE;
        break;
      default:
        fprintf(stderr, "Invalid option: %s\n", argv[i]);
        return 1;
      }
    } else {
      static int first_file = 1;
      if (first_file) {
        fp_in = fopen(argv[i], "r");
        input_file_name = argv[i];
        first_file = 0;
        if (fp_in == NULL) {
          fprintf(stderr, "Error: Invalid file or path: %s\n", argv[i]);
          return 2;
        }
      } else {
          fprintf(stderr, "Error: Only one input file can be processed\n");
          return 2;
      }
    }
  }

  if (fp_in == NULL) {
    fprintf(stderr, "Specify an input file\n"
            "Usage: wave [flags] <file>\n");
    return 2;
  }

  // Print wave file header by default if no
  //  flags are given 
  if (argc == 2 && fp_in)
    flags |= PRINT;

  union header_data *header;
  short *data;
  
  header = (union header_data *) malloc(sizeof(union header_data));
  read_header(fp_in, header, input_file_name);

  if (flags & PRINT)
    print_header_data(header);

  if (flags & WRITE)
    data = read_data(fp_in, header);
  
  if (flags & REVERSE)
    reverse_data(data, header);

  if (flags & MUTE_LEFT)
    mute_left(data, header);

  if (flags & MUTE_RIGHT)
    mute_right(data, header);

  if (flags & CONTROL_VOLUME)
    control_volume(data, header);

  if (flags & WRITE) {
    if (!output_file_name)
      output_file_name = "output.wav";
    fp_out = fopen(output_file_name, "w"); 
    write_wav(fp_out, header, data, output_file_name);
  }
 / 
  fclose(fp_in);
  free(header);
  if (fp_out)
    fclose(fp_out);
  if (flags & WRITE)
    free(data);
  */

