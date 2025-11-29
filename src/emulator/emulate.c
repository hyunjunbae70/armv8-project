#include <stdio.h>
#include <stdlib.h>

#include "machine.h"

/* machine definition */
machine_t machine = {0};

int main(int argc, char **argv) {
  if (argc != 2 && argc != 3) {
    fprintf(stderr, "Usage: ./emulator [file_in] [file_out (optional)]");
    return EXIT_FAILURE;
  }

  const char *filename = argv[1];
  char *outname = NULL;
  if (argc == 3)
    outname = argv[2];

  FILE *outstream = stdout;

  if (outname != NULL)
    outstream = fopen(outname, "w");

  /* load image file */
  machine_load_program(&machine, filename);

  run_machine(&machine);

  /* cleanup */;
  shutdown_machine(&machine, outstream);
  /* IMPORTANT: close the output stream *AFTER* the machine shutdown */
  if (outname != NULL)
    fclose(outstream);

  return EXIT_SUCCESS;
}
