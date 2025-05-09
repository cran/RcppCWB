/*
 *  IMS Open Corpus Workbench (CWB)
 *  Copyright (C) 1993-2006 by IMS, University of Stuttgart
 *  Copyright (C) 2007-     by the respective contributers (see file AUTHORS)
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 2, or (at your option) any later
 *  version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
 *  Public License for more details (in the file "COPYING", or available via
 *  WWW at http://www.gnu.org/copyleft/gpl.html).
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

#include <dirent.h>

#include "../cl/cl.h"
#include "../cl/cwb-globals.h"
#include "../cl/attributes.h"
#include "../cl/ui-helpers.h"

#include "cqp.h"
#include "options.h"
#include "print-modes.h"
#include "output.h"
#include "corpmanag.h"
#include "concordance.h"


#define DEFAULT_EXTERNAL_SORTING_COMMAND \
   "sort -k 2 -k 1n "

#define DEFAULT_EXTERNAL_GROUPING_COMMAND \
   "sort %s -k 1,1n -k 2,2n | uniq -c | sort -k 1,1nr -k 2,2n -k 3,3n"


/*
 * OPTION VARIABLES
 * ================
 *
 * Externals declarations are in the options.h file.
 */


/** Custom enum which lets us know which app we're using; set in each app binary's  main() function */
which_app_t which_app;



/* the insecure/inhibit_activation/inhibit_interactives options aren't really needed any more;
 * CGI scripts should use the new query lock mode instead.
 * [ insecure and inhibit_activation are kept for compatibility; inhibit_interactives has been removed. ]
 */
int insecure;                     /**< Boolean: true means we should not allow pipes etc. (For example, in CGI.) */
int inhibit_activation;           /**< Boolean: true means inhibit corpus activations in parser */


/* debugging options */
int parse_only;                    /**< if true, queries are only parsed, not evaluated. */
int verbose_parser;               /**< if true, absolutely all messages from the parser get printed (inc Message-level). */
int show_symtab;                  /**< at present has no effect (presumably, at one point caused a symbol table to be displayed) */
int show_gconstraints;            /**< if true, the tree of global contraints is printed when an EvalEnvironment is displayed */
int show_evaltree;                /**< if true, the evaluation tree is printed when an EvalEnvironment is displayed */
int show_patlist;                 /**< if true, the pattern list is printed when an EvalEnvironment is displayed */
int show_compdfa;                 /**< if true, the complete DFA is printed when an EvalEnvironment is displayed */
int show_dfa;                     /**< if true, the regex2dfa module will print out the states of the DFA after it is parsed. */
int symtab_debug;                 /**< if this AND debug_simulation are true, print extra messages relating to eval
                                   *   environment labels when simulating an NFA. */
int parser_debug;                 /**< if true, the parser's internal Bison-generated debug setting is turned on. */
int tree_debug;                   /**< if true, extra messages are embedded when an evaluation tree is pretty-printed */
int eval_debug;                   /**< if true, assorted debug messages related to query evaluation are printed */
int search_debug;                 /**< if true, the evaltree of a pattern is pretty-printed before the DFA is created. */
int initial_matchlist_debug;      /**< if true, debug messages relating to the initial set of candidate matches are printed. */
int simulate_debug;               /**< if true, debug messages are printed when simulating an NFA. @see simulate */
int activate_cl_debug;            /**< if true, the CL's debug message setting is set to On. */

/* CQPserver options */
int server_log;                   /**< cqpserver option: logging (print log messages to standard output) */
int server_debug;                 /**< cqpserver option: debugging output (print debug messages to standard error) */
int snoop;                        /**< cqpserver option: monitor CQi network communication (print on SEND/READ messages to standard error) */
int private_server;               /**< cqpserver option: makes CQPserver accept a single connection only */
int server_port;                  /**< cqpserver option: CQPserver's listening port (if 0, listens on CQI_PORT) */
int localhost;                    /**< cqpserver option: accept local connections (loopback) only */
int server_quit;                  /**< cqpserver option: spawn server and return to caller (for CQI::Server.pm) */

int query_lock;                   /**< cqpserver option: safe mode for network/HTTP servers (allow query execution only) */
int query_lock_violation;         /**< cqpserver option: set for CQPserver's sake to detect attempted query lock violation */


/* macro options */
int enable_macros;                /**< enable macros only at user request in case they introduce compatibility problems */
int macro_debug;                  /**< enable debugging of macros (and print macro hash stats on shutdown). */

/* query options */
int hard_boundary;                /**< Query option: use implicit 'within' clause (unless overridden by explicit spec) */
int hard_cut;                     /**< Query option: use hard cut value for all queries (cannot be changed) */
int auto_subquery;                /**< Query option: use auto-subquery mode */
char *def_unbr_attr;              /**< Query option: unbracketed attribute (attribute matched by "..." patterns) */
int query_optimize;               /**< Query option: use query optimisation (untested and expensive optimisations) */
int anchor_number_target;         /**< Query option: which marker @0 ... @9 will be mapped to the target anchor */
int anchor_number_keyword;        /**< Query option: which marker @0 ... @9 will be mapped to the keyword anchor */
MatchingStrategy matching_strategy; /**< Query option: which matching strategy to use */

char *matching_strategy_name;     /**< The matching strategy option is implemented as this string option with side-effect of setting the actual MatchingStrategy*/
int strict_regions;               /**< boolean: expression between {s} ... {/s} tags is constrained to single {s} region  */

/* CQP user interface options */
int use_readline;                 /**< UI option: use GNU Readline for input line editing if available */
int highlighting;                 /**< UI option: highlight match / fields in terminal output? (default = yes) */
int paging;                       /**< UI option: activate/deactivate paging of query results */
char *pager;                      /**< UI option: pager program to used for paged kwic display */
char *tested_pager;               /**< UI option: CQP tests if selected pager works & will fall back to "more" if it doesn't */
char *less_charset_variable;      /**< UI option: name of environment variable for controlling less charset (usually LESSCHARSET) */
int use_colour;                   /**< UI option: use colours for terminal output (experimental) */
int progress_bar;                 /**< UI option: show progress bar during query execution */
int pretty_print;                 /**< UI option: pretty-print most of CQP's output (turn off to simplify parsing of CQP output) */
int autoshow;                     /**< UI option: show query results after evaluation (otherwise, just print number of matches) */
int timing;                       /**< UI option: time queries (printed after execution) */

/* kwic display options */
int show_tag_attributes;          /**< kwic option: show values of s-attributes as SGML tag attributes in kwic lines */
int show_targets;                 /**< kwic option: show numbers of target anchors in brackets */
char *printModeString;            /**< kwic option: string of current printmode (the actual setting is placed in GlobalPrintMode) */
char *printModeOptions;           /**< kwic option: some printing options */
int printNrMatches;               /**< kwic option: -> 'cat' prints number of matches in first line (do we need this?) */
char *printStructure;             /**< kwic option: show annotations of structures containing match */
char *left_delimiter;             /**< kwic option: the match start prefix (defaults to '<'); only affects ascii printing mode */
char *right_delimiter;            /**< kwic option: the match end suffix   (defaults to '>'); only affects ascii printing mode */
char *attribute_separator;        /**< kwic option: override standard attribute separator '/' (if not empty string "") */
char *token_separator;            /**< kwic option: override standard token separator ' ' (if not empty string "") */
char *structure_delimiter;        /**< kwic option: delimiter that is added around structure tags (defaults to empty string "") */

/* files and directories */
char *registry;                   /**< registry directory */
char *data_directory;            /**< directory where subcorpora are stored (saved & loaded) */
int auto_save;                    /**< automatically save subcorpora */
int save_on_exit;                 /**< save unsaved subcorpora upon exit */
char *cqp_init_file;              /**< changed from 'init_file' because of clash with a # define in {term.h} */
char *macro_init_file;            /**< secondary init file for loading macro definitions (not read if macros are disabled) */
char *cqp_history_file;           /**< filename where CQP command history will be saved */
int write_history_file;           /**< Controls whether CQP command history is written to file */

/* options for non-interactive use */
int batchmode;                    /**< set by -f {file} option (don't read ~/.cqprc, then process input from {file}) */
int silent;                       /**< Disables some messages & warnings (used rather inconsistently).
                                   *   NEW: suppresses cqpmessage() unless it is an error. */
char *default_corpus;             /**< corpus specified with -D {corpus} */
char *query_string;               /**< query specified on command line (-E {string}, cqpcl only) */

/* options which just shouldn't exist */
int UseExternalSort;              /**< (option which should not exist) use external sorting algorithm */
char *ExternalSortCommand;        /**< (option which should not exist) external sort command to use */
int UseExternalGroup;             /**< (option which should not exist) use external grouping algorithm */
char *ExternalGroupCommand;       /**< (option which should not exist) external group command to use */
int user_level;                   /**< user level: no longer has any effect on anything, but can still be set (for backwards compatibility) */
int output_binary_ranges;         /**< (option which should not exist) print binary cpos pairs instead of concordance. */


/**
 * Global array of option definitions for CQP.
 */
CQPOption cqpoptions[] = {

  /* Abbr  OptionName             Type        Where to store           String-       Int- Environ Side-  Option
   *                                                                   Default       Def. VarName Effect Flags
   * ----------------------------------------------------------------------------------------------------------*/

  /* debugging options (anything that controls whether or not a message of some kind is printed) */
  { NULL, "VerboseParser",        OptBoolean, &verbose_parser,         NULL,         0,   NULL,   0,     0 },
  { NULL, "ShowSymtab",           OptBoolean, &show_symtab,            NULL,         0,   NULL,   0,     0 },
  { NULL, "ShowGconstraints",     OptBoolean, &show_gconstraints,      NULL,         0,   NULL,   0,     0 },
  { NULL, "ShowEvaltree",         OptBoolean, &show_evaltree,          NULL,         0,   NULL,   0,     0 },
  { NULL, "ShowPatlist",          OptBoolean, &show_patlist,           NULL,         0,   NULL,   0,     0 },
  { NULL, "ShowDFA",              OptBoolean, &show_dfa,               NULL,         0,   NULL,   0,     0 },
  { NULL, "ShowCompDFA",          OptBoolean, &show_compdfa,           NULL,         0,   NULL,   0,     0 },
  { NULL, "SymtabDebug",          OptBoolean, &symtab_debug,           NULL,         0,   NULL,   0,     0 },
  { NULL, "ParserDebug",          OptBoolean, &parser_debug,           NULL,         0,   NULL,   0,     0 },
  { NULL, "TreeDebug",            OptBoolean, &tree_debug,             NULL,         0,   NULL,   0,     0 },
  { NULL, "EvalDebug",            OptBoolean, &eval_debug,             NULL,         0,   NULL,   0,     0 },
  { NULL, "InitialMatchlistDebug",OptBoolean, &initial_matchlist_debug,NULL,         0,   NULL,   0,     0 },
  { NULL, "DebugSimulation",      OptBoolean, &simulate_debug,         NULL,         0,   NULL,   0,     0 },
  { NULL, "SearchDebug",          OptBoolean, &search_debug,           NULL,         0,   NULL,   0,     0 },
  { NULL, "ServerLog",            OptBoolean, &server_log,             NULL,         1,   NULL,   0,     0 },
  { NULL, "ServerDebug",          OptBoolean, &server_debug,           NULL,         0,   NULL,   0,     0 },
  { NULL, "Snoop",                OptBoolean, &snoop,                  NULL,         0,   NULL,   0,     0 },
  { NULL, "Silent",               OptBoolean, &silent,                 NULL,         0,   NULL,   0,     0 },
  { NULL, "ChildProcess",         OptBoolean, &child_process,          NULL,         0,   NULL,   0,     0 },
  { NULL, "MacroDebug",           OptBoolean, &macro_debug,            NULL,         0,   NULL,   0,     0 },
  { NULL, "CLDebug",              OptBoolean, &activate_cl_debug,      NULL,         0,   NULL,   4,     0 },
  /* this last one is a debug option, but doesn't cause printing. */
  { NULL, "ParseOnly",            OptBoolean, &parse_only,             NULL,         0,   NULL,   0,     0 },

  /* "secret" internal options */
  { NULL, "PrintNrMatches",       OptInteger, &printNrMatches,         NULL,         0,   NULL,   0,     0 },

  { "eg", "ExternalGroup",        OptBoolean, &UseExternalGroup,       NULL,         0,   NULL,   0,     0 }, /* should not exist! */
  { "egc","ExternalGroupCommand", OptString,  &ExternalGroupCommand,   NULL,         0,   NULL,   0,     0 }, /* should not exist! */
  { "lcv","LessCharsetVariable",  OptString,  &less_charset_variable,  "LESSCHARSET",0,   NULL,   0,     0 },

  /* options set by command-line flags */
  { NULL, "Readline",             OptBoolean, &use_readline,           NULL,         0,   NULL,   0,     0 },
  { "dc", "DefaultCorpus",        OptString,  &default_corpus,         NULL,         0,   NULL,   0,     0 },
  { "hb", "HardBoundary",         OptInteger, &hard_boundary,          NULL,         DEFAULT_HARDBOUNDARY,
                                                                                          NULL,   0,     0 },
  { "hc", "HardCut",              OptInteger, &hard_cut,               NULL,         0,   NULL,   0,     0 },
  { "ql", "QueryLock",            OptInteger, &query_lock,             NULL,         0,   NULL,   0,     0 },
  { "m",  "Macros",               OptBoolean, &enable_macros,          NULL,         1,   NULL,   0,     0 },
  { NULL, "UserLevel",            OptInteger, &user_level,             NULL,         0,   NULL,   0,     0 }, /* can be set, but has no effect */

  /* Abbr  VariableName           Type        Where to store           String-       Int- Environ Side-  Option
   *                                                                   Default       Def. VarName Effect Flags
   * ----------------------------------------------------------------------------------------------------------*/

  /* now called DataDirectory, but we keep the old name (secretly) for compatibility */
  { "lcd","LocalCorpusDirectory", OptString,  &data_directory,         NULL,         0,   NULL,   2,     0 },

  /* user options: assumed to be settable via the interface (thus the use of the OPTION_VISIBLE_IN_CQP flag that makes them visible). */
  { "r",  "Registry",             OptString,  &registry,               NULL,         0,   CWB_REGISTRY_ENVVAR,
                                                                                                  1,     OPTION_VISIBLE_IN_CQP },
  { "dd", "DataDirectory",        OptString,  &data_directory,         NULL,         0,   DEFAULT_LOCAL_PATH_ENV_VAR,
                                                                                                  2,     OPTION_VISIBLE_IN_CQP },
  { "hf", "HistoryFile",          OptString,  &cqp_history_file,       NULL,         0,   NULL,   0,     OPTION_VISIBLE_IN_CQP },
  { "wh", "WriteHistory",         OptBoolean, &write_history_file,     NULL,         0,   NULL,   0,     OPTION_VISIBLE_IN_CQP },
  { "ms", "MatchingStrategy",     OptString,  &matching_strategy_name, "standard",   0,   NULL,   9,     OPTION_VISIBLE_IN_CQP },
  { "sr", "StrictRegions",        OptBoolean, &strict_regions,         NULL,         1,   NULL,   0,     OPTION_VISIBLE_IN_CQP },
  { "p",  "Paging",               OptBoolean, &paging,                 NULL,         1,   NULL,   0,     OPTION_VISIBLE_IN_CQP },
#ifndef __MINGW__
  { "pg", "Pager",                OptString,  &pager,                  "less -FRX -+S",
                                                                                     0,   "CQP_PAGER",
                                                                                                  0,     OPTION_VISIBLE_IN_CQP },
  { "h",  "Highlighting",         OptBoolean, &highlighting,           NULL,         1,   NULL,   0,     OPTION_VISIBLE_IN_CQP },
#else
  /* use more as default pager under Windows (because it exists whereas less may not :-P ) */
  { "pg", "Pager",                OptString,  &pager,                  "more",       0,   "CQP_PAGER",0, OPTION_VISIBLE_IN_CQP},
  /* this implies that the default value for highlighting must be "off" under Windows */
  { "h",  "Highlighting",         OptBoolean, &highlighting,           NULL,         0,   NULL,   0,     OPTION_VISIBLE_IN_CQP },
#endif
  { "col","Colour",               OptBoolean, &use_colour,             NULL,         0,   NULL,   0,     OPTION_VISIBLE_IN_CQP },
  { "pb", "ProgressBar",          OptBoolean, &progress_bar,           NULL,         0,   NULL,   0,     OPTION_VISIBLE_IN_CQP },
  { "pp", "PrettyPrint",          OptBoolean, &pretty_print,           NULL,         1,   NULL,   0,     OPTION_VISIBLE_IN_CQP },
  { "c",  "Context",              OptContext, &CD,                     NULL,         0,   NULL,   0,     OPTION_VISIBLE_IN_CQP },
  { "lc", "LeftContext",          OptContext, &CD,                     NULL,         0,   NULL,   0,     OPTION_VISIBLE_IN_CQP },
  { "rc", "RightContext",         OptContext, &CD,                     NULL,         0,   NULL,   0,     OPTION_VISIBLE_IN_CQP },
  { "ld", "LeftKWICDelim",        OptString,  &left_delimiter,         "<",          0,   NULL,   0,     OPTION_VISIBLE_IN_CQP },
  { "rd", "RightKWICDelim",       OptString,  &right_delimiter,        ">",          0,   NULL,   0,     OPTION_VISIBLE_IN_CQP },
  { "pm", "PrintMode",            OptString,  &printModeString,        "ascii",      0,   NULL,   6,     OPTION_VISIBLE_IN_CQP },
  { NULL, "AttributeSeparator",   OptString,  &attribute_separator,    "",           0,   NULL,   0,     OPTION_VISIBLE_IN_CQP },
  { NULL, "TokenSeparator",       OptString,  &token_separator,        "",           0,   NULL,   0,     OPTION_VISIBLE_IN_CQP },
  { NULL, "StructureDelimiter",   OptString,  &structure_delimiter,    "",           0,   NULL,   0,     OPTION_VISIBLE_IN_CQP },
  { "po", "PrintOptions",         OptString,  &printModeOptions,       NULL,         0,   NULL,   8,     OPTION_VISIBLE_IN_CQP },
  { "ps", "PrintStructures",      OptString,  &printStructure,         NULL,         0,   NULL,   7,     OPTION_VISIBLE_IN_CQP },
  { "sta","ShowTagAttributes",    OptBoolean, &show_tag_attributes,    NULL,         1,   NULL,   0,     OPTION_VISIBLE_IN_CQP },
  { "st", "ShowTargets",          OptBoolean, &show_targets,           NULL,         0,   NULL,   0,     OPTION_VISIBLE_IN_CQP },
  { "as", "AutoShow",             OptBoolean, &autoshow,               NULL,         1,   NULL,   0,     OPTION_VISIBLE_IN_CQP },
  { NULL, "Timing",               OptBoolean, &timing,                 NULL,         0,   NULL,   0,     OPTION_VISIBLE_IN_CQP },
  { "o",  "Optimize",             OptBoolean, &query_optimize,         NULL,         0,   NULL,   3,     OPTION_VISIBLE_IN_CQP },
  { "ant","AnchorNumberTarget",   OptInteger, &anchor_number_target,   NULL,         0,   NULL,   0,     OPTION_VISIBLE_IN_CQP },
  { "ank","AnchorNumberKeyword",  OptInteger, &anchor_number_keyword,  NULL,         1,   NULL,   0,     OPTION_VISIBLE_IN_CQP },
  { "es", "ExternalSort",         OptBoolean, &UseExternalSort,        NULL,         0,   NULL,   0,     OPTION_VISIBLE_IN_CQP }, /* should not exist! */
  { "esc","ExternalSortCommand",  OptString,  &ExternalSortCommand,    NULL,         0,   NULL,   0,     OPTION_VISIBLE_IN_CQP }, /* should not exist! */
  { "da", "DefaultNonbrackAttr",  OptString,  &def_unbr_attr,          CWB_DEFAULT_ATT_NAME,
                                                                                     0,   NULL,   0,     OPTION_VISIBLE_IN_CQP },
  { "sub","AutoSubquery",         OptBoolean, &auto_subquery,          NULL,         0,   NULL,   0,     OPTION_VISIBLE_IN_CQP },
  { NULL, "AutoSave",             OptBoolean, &auto_save,              NULL,         0,   NULL,   0,     OPTION_VISIBLE_IN_CQP },
  { NULL, "SaveOnExit",           OptBoolean, &save_on_exit,           NULL,         0,   NULL,   0,     OPTION_VISIBLE_IN_CQP },

  /* empty option to terminate array */
  { NULL, NULL,                   OptString,  NULL,                    NULL,         0,   NULL,   0,     0}
};



/*
 * NON-OPTION GLOBAL VARIABLES
 */


/**
 * Child process mode (used by Perl interface (CQP.pm) and by CQPweb (cqp.php))
 *  - don't automatically read in user's .cqprc and .cqpmacros
 *  - print CQP version on startup
 *  - now: output blank line after each command -> SHOULD BE CHANGED
 *  - command ".EOL.;" prints special line (``-::-EOL-::-''), which parent can use to recognise end of output
 *  - print message CQP_PARSE_ERROR_LINE i.e. "PARSE ERROR" on STDERR when a parse error occurs (which parent can easily recognise)
 *
 * This global variable is a Boolean: child process mode on or off.
 */
int child_process;

/** The global context descriptor, used in all the different print modules. */
ContextDescriptor CD;

int handle_sigpipe;

char *progname;
char *licensee;

FILE *batchfh;


/**
 * Generates a resolved path toa file or directory for use as a string-type option,
 * by replacing '~' and '$VAR' with the appropriate environment variables.
 *
 * The return value is a newly-allocated string.
 */
char *
expand_filename(char *fname)
{
  char fn[CL_MAX_FILENAME_LENGTH];
  char *home;
  int s = 0, t = 0;

  while (fname[s] ) {
    /* insert value of home folder for '~' */
    if (fname[s] == '~' && NULL != (home = getenv("HOME"))) {
      int k;
      for (k = 0; home[k]; k++, t++)
        fn[t] = home[k];
      s++;
    }
    /* insert value for an env variable */
    else if (fname[s] == '$') {
      int rpos = 0;
      char rname[CL_MAX_LINE_LENGTH];
      char *reference;

      s++;                        /* skip the $ */
      while (isalnum(fname[s]) || fname[s] == '_')
        rname[rpos++] = fname[s++];
      rname[rpos] = '\0';

      if (!(reference = getenv(rname))) {
        Rprintf("options: can't get value of environment variable ``%s''\n", rname);
        fn[t++] = '$';
//        reference = &rname[0];
        reference = rname;
      }

      for (rpos = 0; reference[rpos]; rpos++, t++)
        fn[t] = reference[rpos];
    }
    /* just insert the character as-is */
    else
      fn[t++] = fname[s++];
  }

  fn[t] = '\0';

  return cl_strdup(fn);
}

/**
 * Prints the usage message for the different CQP applications
 * to standard error and then shuts down the program with exit status 1.
 */
void
cqp_usage(void)
{
  switch (which_app) {
  case cqpserver:
    Rprintf("Usage: %s [options] [<user>:<password> ...]\n", progname);
    break;
  case cqpcl:
    Rprintf("Usage: %s [options] '<query>'\n", progname);
    break;
  case cqp:
    Rprintf("Usage: %s [options]\n", progname);
    break;
  default:
#ifndef R_PACKAGE
    Rprintf("??? Unknown application ???\n");
    exit(cqp_error_status ? cqp_error_status : 1);
#else
    Rf_error("??? Unknown application ???\n");
#endif
  }
  Rprintf("Options:\n");
  Rprintf("    -h           help\n");
  Rprintf("    -v           version and copyright information\n");
  Rprintf("    -r dir       use <dir> as default registry\n");
  Rprintf("    -l dir       store/load subcorpora in <dir>\n");
  Rprintf("    -I file      read <file> as init file\n");
  Rprintf("    -M file      read macro definitions from <file>\n");
  Rprintf("    -m           disable macro expansion\n");
  if (which_app == cqpcl)
    Rprintf("    -E variable  execute query in $(<variable>)\n");
  if (which_app == cqp) {
    Rprintf("    -e           enable input line editing\n");
#ifndef __MINGW__
    Rprintf("    -C           enable ANSI colours (experimental)\n");
#endif
    Rprintf("    -f filename  execute commands from file (batch mode)\n");
    Rprintf("    -p           turn pager off\n");
    Rprintf("    -P pager     use program <pager> to display query results\n");
  }
  if (which_app != cqpserver) {
    Rprintf("    -s           auto auto_subquery mode\n");
    Rprintf("    -c           child process mode\n");
    Rprintf("    -i           print matching ranges only (binary output)\n");
    Rprintf("    -W num       show <num> chars to the left & right of match\n");
    Rprintf("    -L num       show <num> chars to the left of match\n");
    Rprintf("    -R num       show <num> chars to the right of match\n");
  }
  Rprintf("    -D corpus    set default corpus to <corpus>\n");
  Rprintf("    -b num       set hard boundary for kleene star to <num> tokens\n");
  Rprintf("    -S           SIG_PIPE handler toggle\n");
  Rprintf("    -x           insecure mode (when run SETUID)\n");
  if (which_app == cqpserver) {
    Rprintf("    -1           single client server (exits after 1 connection)\n");
    Rprintf("    -P  port     listen on port #<port> [default=CQI_PORT]\n");
    Rprintf("    -L           accept connections from localhost only (loopback)\n");
    Rprintf("    -q           fork() and quit before accepting connections\n");
  }
  Rprintf("    -d mode      activate/deactivate debug mode, where <mode> is one of: \n");
  Rprintf("       [ ShowSymtab, ShowPatList, ShowEvaltree, ShowDFA, ShowCompDFA,   ]\n");
  Rprintf("       [ ShowGConstraints, SymtabDebug, TreeDebug, CLDebug,             ]\n");
  Rprintf("       [ EvalDebug, InitialMatchlistDebug, DebugSimulation,             ]\n");
  Rprintf("       [ VerboseParser, ParserDebug, ParseOnly, SearchDebug, MacroDebug ]\n");
  if (which_app == cqpserver)
    Rprintf("       [ ServerLog [on], ServerDebug, Snoop (log all network traffic)   ]\n");
  Rprintf("       [ ALL (activate all modes except ParseOnly)                      ]\n");
  Rprintf("\n");
#ifndef R_PACKAGE
  exit(cqp_error_status ? cqp_error_status : 1);
#else
  Rf_error("Aborting ...\n");
#endif
}

/**
 * Print the value of a CQP option to stdout.
 * @param opt  Index in the global array of the option to print.
 */
void
print_option_value(int opt)
{
  int show_lc_rc = 0;                /* "set context;" should also display left and right context settings */

  if (cqpoptions[opt].opt_abbrev)
    Rprintf("[%s]\t", cqpoptions[opt].opt_abbrev);
  else
    Rprintf("\t");
  Rprintf("%-22s", cqpoptions[opt].opt_name);

  if (cqpoptions[opt].address) {
    Rprintf("=  ");
    switch (cqpoptions[opt].type) {

    case OptString:
      if (cl_str_is(cqpoptions[opt].opt_name, "PrintOptions"))
        Rprintf("%ctbl %chdr %cwrap %cbdr %cnum",
               GlobalPrintOptions.print_tabular ? '+' : '-',
               GlobalPrintOptions.print_header  ? '+' : '-',
               GlobalPrintOptions.print_wrap    ? '+' : '-',
               GlobalPrintOptions.print_border  ? '+' : '-',
               GlobalPrintOptions.number_lines  ? '+' : '-');
      else if (*((char **)cqpoptions[opt].address)) {
        if ( (cl_str_is(cqpoptions[opt].opt_name, "AttributeSeparator") || cl_str_is(cqpoptions[opt].opt_name, "TokenSeparator"))
            && '\0' == **((char **)cqpoptions[opt].address))
          Rprintf("<default>");
        else if (cl_str_is(cqpoptions[opt].opt_name, "StructureDelimiter") && '\0' == **((char **)cqpoptions[opt].address))
          Rprintf("<none>");
        else
          Rprintf("%s", *((char **)cqpoptions[opt].address));
      }
      else
        Rprintf("<no value>");
      break;

    case OptBoolean:
      Rprintf((*((int *)cqpoptions[opt].address)) ? "yes" : "no");
      break;

    case OptInteger:
      Rprintf("%d", *((int *)cqpoptions[opt].address));
      break;

    case OptContext:
      if (cl_str_is(cqpoptions[opt].opt_name, "Context")) {
        Rprintf("(see below)");
        show_lc_rc = 1;
      }
      else if (cl_str_is(cqpoptions[opt].opt_name, "LeftContext")) {
        Rprintf("%d ", ((ContextDescriptor *)cqpoptions[opt].address)->left_width);

        switch (((ContextDescriptor *)cqpoptions[opt].address)->left_type) {
        case s_att_context:
        case a_att_context:
          Rprintf("%s",
                 ((ContextDescriptor *)cqpoptions[opt].address)->left_structure_name ?
                 ((ContextDescriptor *)cqpoptions[opt].address)->left_structure_name :
                 "(empty?)");
          break;

        case char_context:
          Rprintf("characters");
          break;

        case word_context:
          Rprintf("words");
          break;
        }
      }
      else if (cl_str_is(cqpoptions[opt].opt_name, "RightContext")) {
        Rprintf("%d ", ((ContextDescriptor *)cqpoptions[opt].address)->right_width);

        switch (((ContextDescriptor *)cqpoptions[opt].address)->right_type) {
        case s_att_context:
        case a_att_context:
          Rprintf("%s",
                 ((ContextDescriptor *)cqpoptions[opt].address)->right_structure_name ?
                 ((ContextDescriptor *)cqpoptions[opt].address)->right_structure_name :
                 "(empty?)");
          break;

        case char_context:
          Rprintf("characters");
          break;

        case word_context:
          Rprintf("words");
          break;
        }
      }
      else
        assert (0 && "Unknown option of type OptContext ???");
      break;

    default:
      Rprintf("WARNING: Illegal Option Type!");
      break;
    }

  }
  else
    /* no address given for option -> this is only LeftContext and RightContext in
       normal mode, so refer people to the Context option */
    Rprintf("<not bound to variable>");

  Rprintf("\n");

  if (show_lc_rc) {
    print_option_value(find_option("LeftContext"));
    print_option_value(find_option("RightContext"));
  }
}

/**
 * Prints out the values of all the CQP configuration options.
 */
void
print_option_values()
{
  int opt;
  int lc_opt = find_option("LeftContext"); /* left and right context are automatically shown together with context option */
  int rc_opt = find_option("RightContext");

  if (!silent)
    Rprintf("Variable settings:\n");

  for (opt = 0; cqpoptions[opt].opt_name; opt++)
    if ( cqpoptions[opt].flags & OPTION_VISIBLE_IN_CQP)
      if (opt != lc_opt && opt != rc_opt)
        print_option_value(opt);
}


/**
 * Sets all the CQP options to their default values.
 */
void
set_default_option_values(void)
{
  int i;
  char *env;

  /* 6502 Assembler was not that bad compared to this ... */

  for (i = 0; cqpoptions[i].opt_name ; i++) {
    if (cqpoptions[i].address) {
      switch(cqpoptions[i].type) {

      case OptString:
        *((char **)cqpoptions[i].address) = NULL;
        /* try environment variable first */
        if (cqpoptions[i].envvar != NULL) {
          env = getenv(cqpoptions[i].envvar);
          if (env)
            *((char **)cqpoptions[i].address) = cl_strdup((char *)getenv(cqpoptions[i].envvar));
        }
        /* otherwise, use internal default if specified */
        if (*((char **)cqpoptions[i].address) == NULL) {
          if (cqpoptions[i].cdefault)
            *((char **)cqpoptions[i].address) = cl_strdup(cqpoptions[i].cdefault);
          else
            *((char **)cqpoptions[i].address) = NULL;
        }
        break;

      case OptInteger:
      case OptBoolean:
        if (cqpoptions[i].envvar != NULL)
          *((int *)cqpoptions[i].address) = (getenv(cqpoptions[i].envvar) == NULL)
            ? cqpoptions[i].idefault
            : atoi(getenv(cqpoptions[i].envvar));
        else
          *((int *)cqpoptions[i].address) = cqpoptions[i].idefault;
        break;

      default:
        break;
      }
    }
  }

  query_string = NULL;
  cqp_init_file = NULL;
  macro_init_file = 0;
  inhibit_activation = 0;
  handle_sigpipe = 1;

  initialize_context_descriptor(&CD);
  CD.left_width = DEFAULT_CONTEXT;
  CD.left_type  = char_context;
  CD.right_width = DEFAULT_CONTEXT;
  CD.right_type  = char_context;

  CD.print_cpos = 1;

  /* settings that should not exist */
  ExternalSortCommand = cl_strdup(DEFAULT_EXTERNAL_SORTING_COMMAND);
  ExternalGroupCommand   = cl_strdup(DEFAULT_EXTERNAL_GROUPING_COMMAND);

  /* CQPserver options */
  private_server = 0;
  server_port = 0;
  server_quit = 0;
  localhost = 0;

  matching_strategy = standard_match;  /* unfortunately, this is not automatically derived from the defaults */

  tested_pager = NULL;                 /* this will be set to the PAGER command if that can be successfully run */


  /* execute some side effects for default values */
  cl_set_debug_level(activate_cl_debug);
  cl_set_optimize(query_optimize);
}


/**
 * Look up matching strategy by name.
 *
 * Returns the appropriate enum code for the selected matching strategy or -1 if the argument is invalid.
 *
 * @param s  Name of the desired matching strategy (case insensitive)
 * @return   Code of the matching strategy, or -1 if s isn't a recognized matching strategy name.
 *           The return value can be assigned to enum type matching_strategy unless it is -1.
 */
int
find_matching_strategy(const char *s)
{
  if (cl_streq_ci(s, "traditional"))
    return traditional;

  else if (cl_streq_ci(s, "shortest"))
    return shortest_match;

  else if (cl_streq_ci(s, "standard"))
    return standard_match;

  else if (cl_streq_ci(s, "longest"))
    return longest_match;

  else {
    Rprintf("invalid matching strategy: %s\n", s);
    return -1;
  }
}

/**
 * Finds the index of an option.
 *
 * Return the index in the global options array of the option with name
 * s. This should be never called from outside.
 *
 * @see      cqpoptions
 * @param s  Name of the option to find (or abbreviation); matched case-insensitively.
 * @return   Index of element in cqpoptions corresponding to the name s,
 *           or -1 if no corresponding option was found.
 */
int
find_option(const char *s)
{
  int i;

  for (i = 0 ; cqpoptions[i].opt_name ; i++)
    if (cl_streq_ci(cqpoptions[i].opt_name, s))
      return i;

  /* if no option of name "s" was found, try abbrevs */
  for (i = 0 ; cqpoptions[i].opt_name ; i++)
    if (cqpoptions[i].opt_abbrev && cl_streq_ci(cqpoptions[i].opt_abbrev, s))
      return i;

  return -1;
}


/**
 * Carries out any "side effects" of setting an option.
 *
 * @param opt  The option that has just been set (index into the cqpoptions array).
 */
static void
execute_side_effects(int opt)
{
  int code;

  switch (cqpoptions[opt].side_effect) {

  case 0:  /* <no side effect> */
    break;
  case 1:  /* set Registry "..."; */
    check_available_corpora(SYSTEM);
    break;
  case 2:  /* set DataDirectory "..."; */
    check_available_corpora(SUB);
    /* this is why setting DataDirectory makes the active corpus be de-activated. */
    break;
  case 3:  /* set Optimize (on | off); */
    cl_set_optimize(query_optimize); /* enable / disable CL optimisations, too */
    break;
  case 4:  /* set CLDebug (on | off); */
    cl_set_debug_level(activate_cl_debug); /* enable / disable CL debugging */
    break;

    /* slot 5 is free */

  case 6:  /* set PrintMode (ascii | sgml | html | latex); */
    if (!printModeString || cl_streq_ci(printModeString, "ascii"))
      GlobalPrintMode = PrintASCII;
    else if (cl_streq_ci(printModeString, "sgml"))
      GlobalPrintMode = PrintSGML;
    else if (cl_streq_ci(printModeString, "html"))
      GlobalPrintMode = PrintHTML;
    else if (cl_streq_ci(printModeString, "latex"))
      GlobalPrintMode = PrintLATEX;
    else {
      cqpmessage(Error, "USAGE: set PrintMode (ascii | sgml | html | latex);\n(Invalid mode given, defaulting to ascii)");
      GlobalPrintMode = PrintASCII;
      cl_free(printModeString);
      printModeString = cl_strdup("ascii");
    }
    break;

  case 7:  /* set PrintStructures "..."; */
    if (CD.printStructureTags)
      DestroyAttributeList(&CD.printStructureTags);
    CD.printStructureTags = ComputePrintStructures(current_corpus);
    break;

  case 8:  /* set PrintOptions "...."; */
    ParsePrintOptions();
    break;

  case 9:  /* set MatchingStrategy ( traditional | shortest | standard | longest ); */
    code = find_matching_strategy(matching_strategy_name);
    if (code < 0) {
      cqpmessage(Error, "USAGE: set MatchingStrategy (traditional | shortest | standard | longest);");
      matching_strategy = standard_match;
      cl_free(matching_strategy_name);
      matching_strategy_name = cl_strdup("standard");
    }
    else
      matching_strategy = code;
    break;

  default:
#ifndef R_PACKAGE
    Rprintf("Unknown side-effect #%d invoked by option %s.\n", cqpoptions[opt].side_effect, cqpoptions[opt].opt_name);
    exit(cqp_error_status ? cqp_error_status : 1);
#else
    Rf_error("Unknown side-effect #%d invoked by option %s.\n", cqpoptions[opt].side_effect, cqpoptions[opt].opt_name);
#endif
  }
}

static int
validate_integer_option_value(int opt, int value)
{
  char *optname = cqpoptions[opt].opt_name;
  int tmp;
  int is_ant = (cl_str_is(optname, "AnchorNumberTarget"));
  int is_ank = (cl_str_is(optname, "AnchorNumberKeyword"));

  if (is_ant || is_ank) {
    if (value < 0 || value > 9) {
      cqpmessage(Warning, "set %s must be integer in range 0 .. 9", optname);
      return 0;
    }
    tmp = is_ant ? anchor_number_keyword : anchor_number_target;
    if (value == tmp) {
      cqpmessage(Warning, "set %s must be different from %s (= %d)", optname, (is_ant ? "AnchorNumberKeyword" : "AnchorNumberTarget"), tmp);
      return 0;
    }
  }
  return 1;
}


/**
 * Sets a string-valued option.
 *
 * An error string (function-internal constant, do NOT free)
 * is returned if the type of the option does not correspond to
 * the function which is called. Upon success, NULL is returned.
 *
 * set_string_option_value does NOT strdup the value!
 *
 * @param opt_name  The name of the option to set.
 * @param value     Its new value: must be a free-able string
 *                  (caller no longer has responsibility for freeing)
 * @return          NULL if all OK; otherwise a string describing the problem.
 */
const char *
set_string_option_value(const char *opt_name, char *value)
{
  int opt = find_option(opt_name);

  if (opt < 0)
    return "No such option";

  if (cqpoptions[opt].type == OptContext)
    return set_context_option_value(opt_name, value, 1);

  if (cqpoptions[opt].type != OptString)
    return "Wrong option type (tried to set integer-valued variable to string value)";

  /* free the old value */
  if (*((char **)cqpoptions[opt].address))
    cl_free(*((char **)cqpoptions[opt].address));

  /* if it's a filesystem path, then store a canonical path instead of what we got passed;
   * everything else gets stored unmodified. */
  if (cl_str_is(cqpoptions[opt].opt_name, "Registry") || cl_str_is(cqpoptions[opt].opt_name, "LocalCorpusDirectory") || cl_str_is(cqpoptions[opt].opt_name, "DataDirectory")) {
    *((char **)cqpoptions[opt].address) = expand_filename(value);
    cl_free(value);
  }
  else
    *((char **)cqpoptions[opt].address) = value;

  execute_side_effects(opt);

  return NULL;
}


/**
 * Sets an integer-valued option.
 *
 * An error string (function-internal constant, do NOT free)
 * is returned if the type of the option does not correspond to
 * the function which is called. Upon success, NULL is returned.
 *
 * @param opt_name  The name of the option to set.
 * @param value     Its new value.
 * @return          NULL if all OK; otherwise a string describing the problem.
 */
const char *
set_integer_option_value(const char *opt_name, int value)
{
  int opt = find_option(opt_name);

  if (opt < 0)
    return "No such option";

  if (cqpoptions[opt].type == OptContext)
    return set_context_option_value(opt_name, NULL, value);

  if (cqpoptions[opt].type != OptInteger && cqpoptions[opt].type != OptBoolean)
    return "Wrong option type (tried to set string-valued variable to integer value)";

  if (validate_integer_option_value(opt, value)) {
    *((int *)cqpoptions[opt].address) = value;
    execute_side_effects(opt);
    return NULL;
  }

  return "Illegal value for this option";
}


/**
 * Sets a Context-type option (left, right, or both).
 *
 * An error string (function-internal constant, do NOT free)
 * is returned if the type of the option does not correspond to
 * the function which is called. Upon success, NULL is returned.
 *
 * @param  opt_name  The name of the option to set.
 * @param  sval      Option value: the string part defining the unit.
 *                   NULL here is interpreted as "ival characters".
 * @param  ival      Option new value: the N of units.
 * @return           NULL if all OK; otherwise a string describing the problem.
 */
const char *
set_context_option_value(const char *opt_name, char *sval, int ival)
{
  int context_type;

  int opt = find_option(opt_name);

  if (opt < 0)
    return "No such option";

  if (cqpoptions[opt].type != OptContext)
    return "Illegal value for this option";

  if (sval == NULL ||
      cl_streq_ci(sval, "character") ||
      cl_streq_ci(sval, "char") ||
      cl_streq_ci(sval, "chars") ||
      cl_streq_ci(sval, "characters"))
    context_type = char_context;
  else if (cl_streq_ci(sval, "word") || cl_streq_ci(sval, "words"))
    context_type = word_context;
  else
    context_type = s_att_context;

  if (cl_streq_ci(opt_name, "LeftContext") || cl_streq_ci(opt_name, "lc")) {
    CD.left_structure = NULL;
    CD.left_type = context_type;
    CD.left_width = ival;
    cl_free(CD.left_structure_name);
    if (context_type == s_att_context)
      CD.left_structure_name = cl_strdup(sval);
  }
  else if (cl_streq_ci(opt_name, "RightContext") || cl_streq_ci(opt_name, "rc")) {
    CD.right_structure = NULL;
    CD.right_type = context_type;
    CD.right_width = ival;
    cl_free(CD.right_structure_name);
    if (context_type == s_att_context)
      CD.right_structure_name = cl_strdup(sval);
  }
  else if (cl_streq_ci(opt_name, "Context") || cl_streq_ci(opt_name, "c")) {
    CD.left_structure = NULL;
    CD.left_type = context_type;
    CD.left_width = ival;
    cl_free(CD.left_structure_name);
    if (context_type == s_att_context)
      CD.left_structure_name = cl_strdup(sval);

    CD.right_structure = NULL;
    CD.right_type = context_type;
    CD.right_width = ival;
    cl_free(CD.right_structure_name);
    if (context_type == s_att_context)
      CD.right_structure_name = cl_strdup(sval);
  }
  else
    return "Illegal value for this option/??";

  execute_side_effects(opt);

  return NULL;
}


/** Turns "silent" on and all debug options "off" (avoid repetition in parse_options()) */
static inline void
impose_silent_options(void)
{
  silent = True;
  /* all the debug options should be here if they cause printing.
   * not affected: parse_only */
  verbose_parser            = show_symtab    = show_gconstraints =
    show_evaltree           = show_patlist   = show_compdfa      =
    show_dfa                = symtab_debug   = parser_debug      =
    tree_debug              = eval_debug     = search_debug      =
    initial_matchlist_debug = simulate_debug = macro_debug       =
    activate_cl_debug       = server_debug   = server_log        =
    snoop                   = False;
  cl_set_debug_level(0);
}


/**
 * Parses program options and sets their default values.
 *
 * @param ac  The program's argc.
 * @param av  The program's argv.
 */
void
parse_options(int ac, char *av[])
{
  extern char *optarg;
  /* extern optind and opterr unused, so don't declare them to keep gcc from complaining */

  int c;
  int opt;
  char *valid_options = "";        /* set these depending on application */

  insecure = 0;

  progname = av[0];
  licensee =
    "\n"
    "The IMS Open Corpus Workbench (CWB)\n"
    "\n"
    "Copyright (C) 1993-2006 by IMS, University of Stuttgart\n"
    "Original developer:       Oliver Christ\n"
    "    with contributions by Bruno Maximilian Schulze\n"
    "Version 3.0 developed by: Stefan Evert\n"
    "    with contributions by Arne Fitschen\n"
    "\n"
    "Copyright (C) 2007-today by the CWB open-source community\n"
    "    individual contributors are listed in source file AUTHORS\n"
    "\n"
    "Download and contact: http://cwb.sourceforge.net/\n"
#ifdef COMPILE_DATE
    "\nCompiled:  " COMPILE_DATE
#endif
#ifdef CWB_VERSION
    "\nVersion:   " CWB_VERSION
#endif
    "\n";

  set_default_option_values();
  switch (which_app) {
  case cqp:
    valid_options = "+b:cCd:D:ef:FhiI:l:L:mM:pP:r:R:sSvW:x";
    break;
  case cqpcl:
    valid_options = "+b:cd:D:E:FhiI:l:L:mM:r:R:sSvW:x";
    break;
  case cqpserver:
    valid_options = "+1b:d:D:FhI:l:LmM:P:qr:Svx";
    break;
  default:
    cqp_usage();
    /* this will display the 'unknown application' message */
  }

  while (EOF != (c = getopt(ac, av, valid_options)))
    switch (c) {

    case '1':
      private_server = 1;
      break;

    case 'q':
      server_quit = 1;
      break;

    case 'x':
      insecure = 1;
      break;

    case 'C':
      use_colour = 1;
      break;

    case 'p':
      paging = 0;
      break;

    case 'D':
      default_corpus = cl_strdup(optarg);
      break;

    case 'E':
      if (!(query_string = getenv(optarg))) {
#ifndef R_PACKAGE
        Rprintf("Environment variable %s has no value, exiting\n", optarg);
        exit(cqp_error_status ? cqp_error_status : 1);
#else
        Rf_error("Environment variable %s has no value, exiting\n", optarg);
#endif
      }
      break;

    case 'r':
      registry = cl_strdup(optarg);
      break;

    case 'l':
      data_directory = cl_strdup(optarg);
      break;

    case 'F':
      inhibit_activation = 1;
      break;

    case 'I':
      cqp_init_file = optarg;
      break;

    case 'm':
      enable_macros = 0;        /* -m = DISABLE macros */
      break;

    case 'M':
      macro_init_file = optarg;
      break;

    case 'P':
      if (which_app == cqpserver)        /* this option used in different ways by cqp & cqpserver */
        server_port = atoi(optarg);
      else
        pager = cl_strdup(optarg);
      break;

    case 'd':
      if (!silent) {
        opt = find_option(optarg);

        if (opt >= 0 && cqpoptions[opt].type == OptBoolean) {
          /* TOGGLE the default value */
          *((int *)cqpoptions[opt].address) = cqpoptions[opt].idefault ? 0 : 1;
          execute_side_effects(opt);
        }
        else if (cl_str_is(optarg, "ALL")) {
          /* set the debug values */
          verbose_parser = show_symtab = show_gconstraints =
            show_evaltree = show_patlist = show_dfa = show_compdfa =
            symtab_debug = parser_debug = eval_debug =
            initial_matchlist_debug = simulate_debug =
            search_debug = macro_debug = activate_cl_debug =
            server_debug = server_log = snoop = True;
          /* execute side effect for CLDebug option */
          cl_set_debug_level(activate_cl_debug);
        }
        else {
#ifndef R_PACKAGE
          Rprintf("Invalid debug mode: -d %s\nType '%s -h' for more information.\n", optarg, progname);
          exit(cqp_error_status ? cqp_error_status : 1);
#else
          Rf_error("Invalid debug mode: -d %s\nType '%s -h' for more information.\n", optarg, progname);
#endif
        }
      }
      break;

    case 'h':
      cqp_usage();
      break;

    case 'v':
#ifndef R_PACKAGE
      Rprintf("%s\n", licensee);
      exit(cqp_error_status);
#else
      Rf_error("%s\n", licensee);
#endif

    case 's':
      auto_subquery = 1;
      break;

    case 'S':
      if (handle_sigpipe)
        handle_sigpipe = 0;
      else
        handle_sigpipe = 1;
      break;

    case 'W':
      CD.left_width = CD.right_width = atoi(optarg);
      execute_side_effects(3);
      break;

    case 'L':
      if (which_app == cqpserver)        /* used in different ways by cqpserver & cqp/cqpcl */
        localhost = True;                /* takes no arg with cqpserver */
      else
        CD.left_width = atoi(optarg);
      break;

    case 'R':
      CD.right_width = atoi(optarg);
      break;

    case 'b':
      hard_boundary = atoi(optarg);
      break;

    case 'i':
      /* print binary cpos begin/end instead of concordance; turn off all debug and silenceable output. */
      output_binary_ranges = True;
      impose_silent_options();
      /* cf. options.h; there are more debug vars than this now, should they all be set to false? */
      break;

    case 'c':
      child_process = True;
      silent = True;
      pretty_print = paging = highlighting = False;
      autoshow = auto_save = False;
      progress_bar_child_mode(1);
      break;

    case 'e':
      use_readline = True;
      break;

    case 'f':
      batchmode = True;
      impose_silent_options();
      /* note that cl_open_stream() handles the case where the filename is "-" for stdin */
      if (!(batchfh = cl_open_stream(optarg, CL_STREAM_READ, CL_STREAM_MAGIC))) {
        perror(optarg);
#ifndef R_PACKAGE
        exit(cqp_error_status ? cqp_error_status : 1);
#else
        Rf_error("Aborting ...\n");
#endif
      }
      break;

    default:
#ifndef R_PACKAGE
      Rprintf("Invalid option. Type '%s -h' for more information.\n", progname);
      exit(cqp_error_status ? cqp_error_status : 1);
#else
      Rf_error("Invalid option. Type '%s -h' for more information.\n", progname);
#endif
      break;
    }
}
