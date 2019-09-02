/*!
\file
\brief Parsing of command-line arguments

\date   Started 3/10/2015
\author George Karypis with contributions by Xia Ning, Athanasios N. Nikolakopoulos, Zeren Shui and Mohit Sharma.
\author Copyright 2019, Regents of the University of Minnesota
*/

#include "slimbin.h"

/*------------------------------------------------------------------------*/
/*! Command-line options */
/*------------------------------------------------------------------------*/
static struct gk_option long_options[] = {{"ifmt", 1, 0, CMD_IFMT},
                                          {"binarize", 0, 0, CMD_BINARIZE},
                                          {"outfile", 1, 0, CMD_OUTFILE},
                                          {"nrcmds", 1, 0, CMD_NRCMDS},
                                          {"dbglvl", 1, 0, CMD_DBGLVL},
                                          {"help", 0, 0, CMD_HELP},
                                          {0, 0, 0, 0}};

/*------------------------------------------------------------------------*/
/*! Mappings for the various parameter values */
/*------------------------------------------------------------------------*/
static gk_StringMap_t ifmt_options[] = {
    {"csr", GK_CSR_FMT_CSR},
    {"csrnv", GK_CSR_FMT_METIS}, /* this will be converted to CSR */
    {"cluto", GK_CSR_FMT_CLUTO},
    {"ijv", GK_CSR_FMT_IJV},
    {NULL, 0}};

/*------------------------------------------------------------------------*/
/*! Mini help */
/*------------------------------------------------------------------------*/
static char helpstr[][512] = {
    " ",
    " Usage:",
    "   slim_predict [options] model-file old-file [test-file]",
    " ",
    " Parameters:",
    "   model-file",
    "       The file that stores the model that was generated by slim_learn.",
    " ",
    "   old-file",
    "       The file that stores the historical information for each user.",
    " ",
    "   test-file",
    "       The file that stores the hidden items for each user.",
    " ",
    " Options:",
    "   -ifmt=string",
    "      Specifies the format of the input files. Available options are:",
    "        csr     -  CSR format [default].",
    "        csrnv   -  CSR format without ratings.",
    "        cluto   -  Format used by CLUTO.",
    "        ijv     -  One (row#, col#, val) per line.",
    " "
    "   -binarize",
    "      Specifies that the ratings should be binarized.",
    " ",
    "   -outfile=string",
    "      Specifies the output file that will store the predictions.",
    "      If not specified, no output will be produced.",
    " ",
    "   -nrcmds=int",
    "      Specifies the number of items to recommend for each user.",
    "      The default value is 10.",
    " ",
    "   -dbglvl=int",
    "      Specifies the debug level. The default value is 0.",
    " ",
    "   -help",
    "      Prints this message.",
    " ",
    ""};

/*------------------------------------------------------------------------*/
/*! A short help */
/*------------------------------------------------------------------------*/
static char shorthelpstr[][100] = {
    " ", " Usage: slim_predict [options] model-file old-file [test-file]",
    "   use 'slim_predict -help' for a summary of the options.", ""};

/**************************************************************************/
/*! Parses command-line arguments */
/**************************************************************************/
params_t *parse_cmdline(int argc, char *argv[]) {
  int c = -1, option_index = -1;
  params_t *params;

  params = (params_t *)gk_malloc(sizeof(params_t), "parse_cmdline");
  memset((void *)params, 0, sizeof(params_t));

  /* setup defaults */
  params->ifmt = GK_CSR_FMT_CSR;
  params->readvals = 1;
  params->binarize = 0;
  params->outfile = NULL;
  params->tstfile = NULL;
  params->nrcmds = 10;
  params->dbglvl = 0;

  while ((c = gk_getopt_long_only(argc, argv, "", long_options,
                                  &option_index)) != -1) {
    switch (c) {
    case CMD_IFMT:
      if ((params->ifmt = gk_GetStringID(ifmt_options, gk_optarg)) == -1)
        errexit("Invalid -ifmt of %s.\n", gk_optarg);
      /* deal with the no-ratings case */
      if (params->ifmt == GK_CSR_FMT_METIS) {
        params->ifmt = GK_CSR_FMT_CSR;
        params->readvals = 0;
      }
      break;

    case CMD_BINARIZE:
      params->binarize = 1;
      break;

    case CMD_OUTFILE:
      params->outfile = gk_strdup(gk_optarg);
      break;

    case CMD_DBGLVL:
      if ((params->dbglvl = atoi(gk_optarg)) < 0)
        errexit("The -dbglvl parameter should be non-negative.\n");
      break;

    case CMD_NRCMDS:
      if ((params->nrcmds = atoi(gk_optarg)) < 0)
        errexit("The -nrcmds parameter should be non-negative.\n");
      break;

    case '?':
    case CMD_HELP:
      for (int i = 0; strlen(helpstr[i]) > 0; i++)
        printf("%s\n", helpstr[i]);
      exit(0);

    default:
      printf("Illegal command-line option(s) %s\n", gk_optarg);
      exit(0);
    }
  }

  /* get the datafile */
  if (argc - gk_optind < 1 || argc - gk_optind > 3) {
    for (int i = 0; strlen(shorthelpstr[i]) > 0; i++)
      printf("%s\n", shorthelpstr[i]);
    exit(0);
  }

  params->mdlfile = gk_strdup(argv[gk_optind++]);
  if (!gk_fexists(params->mdlfile))
    errexit("Input model file %s does not exist.\n", params->mdlfile);

  params->trnfile = gk_strdup(argv[gk_optind++]);
  if (!gk_fexists(params->trnfile))
    errexit("Input old file %s does not exist.\n", params->trnfile);

  if (argc - gk_optind == 1) {
    params->tstfile = gk_strdup(argv[gk_optind++]);
    if (!gk_fexists(params->tstfile))
      errexit("Input test file %s does not exist.\n", params->tstfile);
  }

  return params;
}
