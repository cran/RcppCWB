#' Get Attribute Size (of Positional/Structural Attribute).
#' 
#' Use `cl_attribute_size()` to get the total number of values of a positional
#' attribute (param `attribute_type` = "p"), or structural attribute (param
#' `attribute_type` = "s"). Note that indices are zero-based, i.e. the maximum
#' position of a positional / structural attribute is attribute size minus 1
#' (see examples).
#' @rdname cl_attribute_size
#' @param corpus name of a CWB corpus (upper case)
#' @param attribute name of a p- or s-attribute
#' @param attribute_type either "p" or "s", for structural/positional attribute
#' @param registry path to the registry directory, defaults to the value of the
#'   environment variable CORPUS_REGISTRY
#' @examples 
#' token_no <- cl_attribute_size(
#'   "REUTERS",
#'   attribute = "word",
#'   attribute_type = "p",
#'   registry = get_tmp_registry()
#' )
#' corpus_positions <- seq.int(from = 0, to = token_no - 1)
#' cl_cpos2id(
#'   "REUTERS",
#'   "word",
#'   cpos = corpus_positions,
#'   registry = get_tmp_registry()
#' )
#' 
#' places_no <- cl_attribute_size(
#'   "REUTERS",
#'   attribute = "places",
#'   attribute_type = "s",
#'   registry = get_tmp_registry()
#' )
#' strucs <- seq.int(from = 0, to = places_no - 1)
#' cl_struc2str(
#'   "REUTERS",
#'   "places",
#'   struc = strucs,
#'   registry = get_tmp_registry()
#' )
cl_attribute_size <- function(corpus, attribute, attribute_type, registry = Sys.getenv("CORPUS_REGISTRY")){
  attribute_size(
    corpus = corpus,
    attribute = attribute,
    attribute_type = attribute_type,
    registry = registry
  )
}

#' Get Lexicon Size.
#' 
#' Get the total number of unique tokens/ids of a positional attribute. Note
#' that token ids are zero-based, i.e. when iterating through tokens, start at
#' 0, the maximum will be \code{cl_lexicon_size()} minus 1.
#' 
#' @param corpus name of a CWB corpus (upper case)
#' @param p_attribute name of positional attribute
#' @param registry path to the registry directory, defaults to the value of the
#'   environment variable CORPUS_REGISTRY
#' @rdname cl_lexicon_size
#' @examples 
#' lexicon_size <- cl_lexicon_size(
#'   "REUTERS",
#'   p_attribute = "word",
#'   registry = get_tmp_registry()
#' )
#' 
#' token_ids <- seq.int(from = 0, to = lexicon_size - 1)
#' cl_id2str(
#'   "REUTERS",
#'   p_attribute = "word",
#'   id = token_ids,
#'   registry = get_tmp_registry()
#' )
cl_lexicon_size <- function(corpus, p_attribute, registry = Sys.getenv("CORPUS_REGISTRY")){
  .cl_lexicon_size(corpus = corpus, p_attribute = p_attribute, registry = registry)
}


#' @title Using Structural Attributes.
#' 
#' @description Structural attributes store the metadata of texts in a CWB
#'   corpus and/or any kind of annotation of a region of text. The fundamental
#'   unit are so-called strucs, i.e. indices of regions identified by a left and
#'   a right corpus position. The corpus library (CL) offers a set of functions
#'   to make the translations between corpus positions (cpos) and strucs
#'   (struc).
#' 
#' @param corpus name of a CWB corpus (upper case)
#' @param s_attribute name of structural attribute (character vector)
#' @param cpos An \code{integer} vector with corpus positions.
#' @param struc a struc identifying a region
#' @param registry path to the registry directory, defaults to the value of the
#'   environment variable CORPUS_REGISTRY
#' @rdname s_attributes
#' @name CL: s_attributes
#' @examples
#' # get metadata for matches of token
#' # scenario: id of the texts with occurrence of 'oil'
#' token_to_get <- "oil"
#' token_id <- cl_str2id("REUTERS", p_attribute = "word", str = "oil", get_tmp_registry())
#' token_cpos <- cl_id2cpos("REUTERS", p_attribute = "word", id = token_id, get_tmp_registry())
#' strucs <- cl_cpos2struc("REUTERS", s_attribute = "id", cpos = token_cpos, get_tmp_registry())
#' strucs_unique <- unique(strucs)
#' text_ids <- cl_struc2str("REUTERS", s_attribute = "id", struc = strucs_unique, get_tmp_registry())
#' 
#' # get the full text of the first text with match for 'oil'
#' left_cpos <- cl_cpos2lbound(
#'   "REUTERS", s_attribute = "id",
#'   cpos = min(token_cpos),
#'   registry = get_tmp_registry()
#' )
#' right_cpos <- cl_cpos2rbound(
#'   "REUTERS",
#'   s_attribute = "id",
#'   cpos = min(token_cpos),
#'   registry = get_tmp_registry()
#' )
#' txt <- cl_cpos2str(
#'   "REUTERS", p_attribute = "word",
#'   cpos = left_cpos:right_cpos,
#'   registry = get_tmp_registry()
#' )
#' fulltext <- paste(txt, collapse = " ")
#' 
#' # alternativ approach to achieve same result
#' first_struc_match_oil <- cl_cpos2struc(
#'   "REUTERS", s_attribute = "id",
#'   cpos = min(token_cpos),
#'   registry = get_tmp_registry()
#'   )
#' cpos_struc <- cl_struc2cpos(
#'   "REUTERS", s_attribute = "id",
#'   struc = first_struc_match_oil,
#'   registry = get_tmp_registry()
#' )
#' txt <- cl_cpos2str(
#'   "REUTERS",
#'   p_attribute = "word",
#'   cpos = cpos_struc[1]:cpos_struc[2],
#'   registry = get_tmp_registry()
#' )
#' fulltext <- paste(txt, collapse = " ")
cl_cpos2struc <- function(corpus, s_attribute, cpos, registry = Sys.getenv("CORPUS_REGISTRY")){
  check_registry(registry)
  check_corpus(corpus, registry, cqp = FALSE)
  check_s_attribute(corpus = corpus, registry = registry, s_attribute = s_attribute)
  
  if (length(cpos) == 0L) return(integer())
  check_cpos(corpus = corpus, p_attribute = "word", cpos = cpos, registry = registry)
  
  .cl_cpos2struc(corpus = corpus, s_attribute = s_attribute, cpos = cpos, registry = registry)
}

#' @rdname s_attributes
cl_struc2cpos <- function(corpus, s_attribute, registry = Sys.getenv("CORPUS_REGISTRY"), struc){
  check_registry(registry)
  check_corpus(corpus, registry, cqp = FALSE)
  check_s_attribute(corpus = corpus, registry = registry, s_attribute = s_attribute)
  check_strucs(corpus = corpus, s_attribute = s_attribute, strucs = struc, registry = registry)
  struc2cpos(corpus = corpus, s_attribute = s_attribute, registry = registry, struc = struc)
}

#' @rdname s_attributes
cl_struc2str <- function(corpus, s_attribute, struc, registry = Sys.getenv("CORPUS_REGISTRY")){
  check_registry(registry)
  check_corpus(corpus, registry, cqp = FALSE)
  check_s_attribute(corpus = corpus, registry = registry, s_attribute = s_attribute)
  check_strucs(corpus = corpus, s_attribute = s_attribute, strucs = struc, registry = registry)
  .cl_struc2str(corpus = corpus, s_attribute = s_attribute, struc = struc, registry = registry)
}


#' @title Using Positional Attributes.
#' 
#' @description CWB indexed corpora store the text of a corpus as numbers: Every token
#' in the token stream of the corpus is identified by a unique corpus
#' position. The string value of every token is identified by a unique integer
#' id. The corpus library (CL) offers a set of functions to make the transitions
#' between corpus positions, token ids, and the character string of tokens.
#' 
#' @param corpus name of a CWB corpus (upper case)
#' @param registry path to the registry directory, defaults to the value of the
#'   environment variable CORPUS_REGISTRY
#' @param p_attribute a p-attribute (positional attribute)
#' @param cpos corpus positions (integer vector)
#' @param id id of a token
#' @param regex a regular expression
#' @param str a character string
#' @rdname p_attributes
#' @name CL: p_attributes
#' @examples 
#' # registry directory and cpos_total will be needed in examples
#' cpos_total <- cl_attribute_size(
#'   corpus = "REUTERS", attribute = "word",
#'   attribute_type = "p", registry = get_tmp_registry()
#'   )
#' 
#' # decode the token stream of the corpus (the quick way)
#' token_stream_str <- cl_cpos2str(
#'   corpus = "REUTERS", p_attribute = "word",
#'   cpos = seq.int(from = 0, to = cpos_total - 1),
#'   registry = get_tmp_registry()
#'   )
#'   
#' # decode the token stream (cpos2id first, then id2str)
#' token_stream_ids <- cl_cpos2id(
#'   corpus = "REUTERS", p_attribute = "word",
#'   cpos = seq.int(from = 0, to = cpos_total - 1),
#'   registry = get_tmp_registry()
#'   )
#' token_stream_str <- cl_id2str(
#'   corpus = "REUTERS", p_attribute = "word",
#'   id = token_stream_ids, registry = get_tmp_registry()
#' )
#' 
#' # get corpus positions of a token
#' token_to_get <- "oil"
#' id_oil <- cl_str2id(
#'   corpus = "REUTERS", p_attribute = "word",
#'   str = token_to_get, registry = get_tmp_registry()
#'   )
#' cpos_oil <- cl_id2cpos <- cl_id2cpos(
#'   corpus = "REUTERS", p_attribute = "word",
#'   id = id_oil, registry = get_tmp_registry()
#' )
#' 
#' # get frequency of token
#' oil_freq <- cl_id2freq(
#'   corpus = "REUTERS", p_attribute = "word", id = id_oil, registry = get_tmp_registry()
#' )
#' length(cpos_oil) # needs to be the same as oil_freq
#' 
#' # use regular expressions 
#' ids <- cl_regex2id(
#'   corpus = "REUTERS", p_attribute = "word",
#'   regex = "M.*", registry = get_tmp_registry()
#' )
#' m_words <- cl_id2str(
#'   corpus = "REUTERS", p_attribute = "word",
#'   id = ids, registry = get_tmp_registry()
#' )
#' 
cl_cpos2str <- function(corpus, p_attribute, registry = Sys.getenv("CORPUS_REGISTRY"), cpos){
  check_registry(registry)
  check_corpus(corpus, registry, cqp = FALSE)
  if (length(cpos) == 0L) return(integer())
  cpos2str(corpus = corpus, p_attribute = p_attribute, registry = registry, cpos = cpos)
}

#' @rdname p_attributes
cl_cpos2id <- function(corpus, p_attribute, registry = Sys.getenv("CORPUS_REGISTRY"), cpos){
  check_registry(registry)
  check_corpus(corpus, registry, cqp = FALSE)
  if (length(cpos) == 0L) return(integer())
  cpos2id(corpus = corpus, p_attribute = p_attribute, registry = registry, cpos = cpos)
}

#' @rdname p_attributes
cl_id2str <- function(corpus, p_attribute, registry = Sys.getenv("CORPUS_REGISTRY"), id){
  check_registry(registry)
  check_corpus(corpus, registry, cqp = FALSE)
  check_id(corpus = corpus, p_attribute = p_attribute, id = id, registry = registry)
  id2str(corpus = corpus, p_attribute = p_attribute, registry = registry, id = id)
}

#' @rdname p_attributes
cl_regex2id <- function(corpus, p_attribute, regex, registry = Sys.getenv("CORPUS_REGISTRY")){
  check_registry(registry)
  check_corpus(corpus, registry, cqp = FALSE)
  .cl_regex2id(corpus = corpus, p_attribute = p_attribute, regex = regex, registry = registry)
}

#' @rdname p_attributes
cl_str2id <- function(corpus, p_attribute, str, registry = Sys.getenv("CORPUS_REGISTRY")){
  check_registry(registry)
  check_corpus(corpus, registry, cqp = FALSE)
  .cl_str2id(corpus = corpus, p_attribute = p_attribute, str = str, registry = registry)
}

#' @rdname p_attributes
cl_id2freq <- function(corpus, p_attribute, id, registry = Sys.getenv("CORPUS_REGISTRY")){
  check_registry(registry)
  check_corpus(corpus, registry, cqp = FALSE)
  check_p_attribute(p_attribute = p_attribute, corpus = corpus, registry = registry)
  check_id(corpus = corpus, p_attribute = p_attribute, id = id, registry = registry)
  .cl_id2freq(corpus = corpus, p_attribute = p_attribute, id = id, registry = registry)
}


#' @rdname p_attributes
cl_id2cpos <- function(corpus, p_attribute, id, registry = Sys.getenv("CORPUS_REGISTRY")){
  check_registry(registry)
  check_corpus(corpus, registry, cqp = FALSE)
  check_p_attribute(p_attribute = p_attribute, corpus = corpus, registry = registry)
  check_id(corpus = corpus, p_attribute = p_attribute, id = id, registry = registry)
  .cl_id2cpos(corpus = corpus, p_attribute = p_attribute, id = id, registry = registry)
}

#' Load corpus.
#' 
#' @inheritParams cl_delete_corpus
#' @return A `externalptr` referencing the C representation of the corpus.
#' @export
cl_find_corpus <- function(corpus, registry){
  .cl_find_corpus(corpus = tolower(corpus), registry = registry)
}

#' Drop loaded corpus.
#' 
#' Remove a corpus from the list of loaded corpora of the corpus library (CL).
#' 
#' The corpus library (CL) internally maintains a list of corpora including
#' information on positional and structural attributes so that the registry file
#' needs not be parsed again and again. However, when an attribute has been
#' added to the corpus, it will not yet be visible, because it is not part of
#' the data that has been loaded. The \code{cl_delete_corpus} function exposes a
#' CL function named identically, to force reloading the corpus (after it has
#' been deleted), which will include parsing an updated registry file.
#' 
#' @param corpus name of a CWB corpus (upper case) 
#' @param registry path to the registry directory, defaults to the value of the
#'   environment variable CORPUS_REGISTRY
#' @return An `integer` value 1 is returned invisibly if a previously loaded
#'   corpus has been deleted, or 0 if the corpus has not been loaded and has not
#'   been deleted.
#' @export cl_delete_corpus
#' @examples
#' cl_attribute_size("UNGA", attribute = "word", attribute_type = "p")
#' corpus_is_loaded("UNGA")
#' cl_delete_corpus("UNGA")
#' corpus_is_loaded("UNGA")
cl_delete_corpus <- function(corpus, registry = Sys.getenv("CORPUS_REGISTRY")){
  as.logical(.cl_delete_corpus(corpus = corpus, registry = registry))
}

#' Get charset of a corpus.
#' 
#' The encoding of a corpus is declared in the registry file (corpus property
#' "charset"). Once a corpus is loaded, this information is available without
#' parsing the registry file again and again. The \code{cl_charset_name} offers
#' a quick access to this information.
#' 
#' @param corpus Name of a CWB corpus (upper case).
#' @param registry Path to the registry directory, defaults to the value of the
#'   environment variable CORPUS_REGISTRY
#' @export cl_charset_name
#' @examples
#' cl_charset_name(
#'   corpus = "REUTERS",
#'   registry = system.file(package = "RcppCWB", "extdata", "cwb", "registry")
#' )
cl_charset_name <- function(corpus, registry = Sys.getenv("CORPUS_REGISTRY")){
  .cl_charset_name(corpus = corpus, registry = registry)
}

#' Check whether structural attribute has values
#' 
#' Structural attributes do not necessarily have values, structural attributes
#' (such as annotations of sentences or paragraphs) may just define regions of
#' corpus positions. Use this function to test whether an attribute has values.
#' 
#' @param corpus Corpus ID, a length-one `character` vector.
#' @param s_attribute Structural attribute to check, a length-one `character` vector.
#' @param registry The registry directory of the corpus.
#' @return `TRUE` if the attribute has values and `FALSE` if not. `NA` if the structural
#'   attribute is not available.
#' @export cl_struc_values
#' @examples
#' cl_struc_values("REUTERS", "places") # TRUE - attribute has values
#' cl_struc_values("REUTERS", "date") # NA - attribute does not exist
cl_struc_values <- function(corpus, s_attribute, registry = Sys.getenv("CORPUS_REGISTRY")){
  check_corpus(corpus = corpus, registry = registry, cqp = FALSE)
  registry <- path(path_expand(registry))
  .cl_struc_values(corpus = corpus, s_attribute = s_attribute, registry = registry)
}

#' Get information from registry file
#' 
#' Extract information from the internal C representation of registry data.
#' @details `corpus_data_dir()` will return the data directory (class `fs_path`)
#'   where the binary files of a corpus are kept (a directory also known as
#'   'home' directory).
#' @param corpus A length-one `character` vector with the corpus ID.
#' @param registry A length-one `character` vector with the registry directory.
#' @export corpus_data_dir
#' @rdname registry_info
#' @importFrom fs path_expand
#' @examples
#' corpus_data_dir("REUTERS", registry = get_tmp_registry())
corpus_data_dir <- function(corpus, registry = Sys.getenv("CORPUS_REGISTRY")){
  check_corpus(corpus = corpus, registry = registry, cqp = FALSE)
  registry <- path(path_expand(registry))
  dir <- .corpus_data_dir(corpus = corpus, registry = registry)
  path(dir)
}

#' @details `corpus_info_file()` will return the path to the info file for a
#'   corpus (class `fs_path` object). If info file does not exist or INFO line
#'   is missing in the registry file, `NA` is returned.
#' @rdname registry_info
#' @examples
#' corpus_info_file("REUTERS", registry = get_tmp_registry())
corpus_info_file <- function(corpus, registry = Sys.getenv("CORPUS_REGISTRY")){
  check_corpus(corpus = corpus, registry = registry, cqp = FALSE)
  registry <- path(path_expand(registry))
  fname <- .corpus_info_file(corpus = corpus, registry = registry)
  path(fname)
}

#' @details `corpus_full_name()` will return the full name of the corpus defined
#'   in the registry file.
#' @rdname registry_info
#' @examples
#' corpus_full_name("REUTERS", registry = get_tmp_registry())
corpus_full_name <- function(corpus, registry = Sys.getenv("CORPUS_REGISTRY")){
  check_corpus(corpus = corpus, registry = registry, cqp = FALSE)
  registry <- path(path_expand(registry))
  .corpus_full_name(corpus = corpus, registry = registry)
}

#' @details `corpus_p_attributes()` returns a `character` vector with the
#'   positional attributes of a corpus.
#' @rdname registry_info
#' @examples
#' corpus_p_attributes("REUTERS", registry = get_tmp_registry())
corpus_p_attributes <- function(corpus, registry = Sys.getenv("CORPUS_REGISTRY")){
  check_corpus(corpus = corpus, registry = registry, cqp = FALSE)
  registry <- path(path_expand(registry))
  .corpus_p_attributes(corpus = corpus, registry = registry)
}

#' @details `corpus_s_attributes()` returns a `character` vector with the
#'   structural attributes of a corpus.
#' @rdname registry_info
#' @examples
#' corpus_s_attributes("REUTERS", registry = get_tmp_registry())
corpus_s_attributes <- function(corpus, registry = Sys.getenv("CORPUS_REGISTRY")){
  check_corpus(corpus = corpus, registry = registry, cqp = FALSE)
  registry <- path(path_expand(registry))
  .corpus_s_attributes(corpus = corpus, registry = registry)
}

#' @details `corpus_properties()` returns a `character` vector with the corpus
#'   properties defined in the registry file. If the corpus cannot be located,
#'   `NA` is returned.
#' @rdname registry_info
#' @examples
#' corpus_properties("REUTERS", registry = get_tmp_registry())
corpus_properties <- function(corpus, registry = Sys.getenv("CORPUS_REGISTRY")){
  check_corpus(corpus = corpus, registry = registry, cqp = FALSE)
  registry <- path(path_expand(registry))
  .corpus_properties(corpus = corpus, registry = registry)
}

#' @details `corpus_property()` returns the value of a corpus property defined
#'   in the registry file, or `NA` if the corpus does not exist, is not loaded 
#'   of if the property requested is undefined.
#' @param property A corpus property defined in the registry file (.
#' @rdname registry_info
#' @examples
#' corpus_property(
#'   "REUTERS",
#'   registry = get_tmp_registry(),
#'   property = "language"
#' )
corpus_property <- function(corpus, registry = Sys.getenv("CORPUS_REGISTRY"), property){
  stopifnot(
    !missing(property),
    length(property) == 1L,
    is.character(property)
  )
  check_corpus(corpus = corpus, registry = registry, cqp = FALSE)
  registry <- path(path_expand(registry))
  .corpus_property(corpus = corpus, registry = registry, property = property)
}


#' @details `corpus_get_registry()` will extract the registry directory with the
#'   registry file defining a corpus from the internal C representation of
#'   loaded corpora. The `character` vector that is returned may be > 1 if there
#'   are several corpora with the same id defined in registry files in different
#'   (registry) directories. If the corpus is not found, `NA` is returned.
#' @rdname registry_info
#' @examples
#' corpus_registry_dir("REUTERS")
#' corpus_registry_dir("FOO") # NA returned
corpus_registry_dir <- function(corpus){
  registry <- .corpus_registry_dir(tolower(corpus))
  if (length(registry) == 1L){
    if (is.na(registry)) registry else path(registry)
  } else {
    path(registry)
  }
}

#' Check whether corpus is loaded
#' 
#' @inheritParams corpus_data_dir
#' @return `TRUE` if corpus is loaded and `FALSE` if not.
#' @export corpus_is_loaded
corpus_is_loaded <- function(corpus, registry = Sys.getenv("CORPUS_REGISTRY")){
  as.logical(.corpus_is_loaded(corpus = corpus, registry = registry))
}

#' Load corpus
#' 
#' @inheritParams corpus_data_dir
#' @return `TRUE` if corpus could be loaded and `FALSE` if not.
#' @export corpus_is_loaded
#' @examples
#' cl_load_corpus("REUTERS")
cl_load_corpus <- function(corpus, registry = Sys.getenv("CORPUS_REGISTRY")){
  as.logical(.cl_load_corpus(corpus = corpus, registry = registry))
}

#' Show CL corpora
#' 
#' @return A `character` vector.
#' @export corpus_is_loaded
#' @examples
#' cl_list_corpora()
cl_list_corpora <- function(){
  .cl_list_corpora()
}


#' Low-level CL access.
#' 
#' Wrappers for CWB Corpus Library functions suited for writing performance
#' code.
#' 
#' The default cl_* R wrappers for the functions of the CWB Corpus Library
#' involve a lookup of a corpus and its p- or s-attributes (using the corpus ID,
#' registry and attribute indicated by length-one character vectors) every time
#' one of these functions is called. It is more efficient looking up an
#' attribute only once. This set of functions passes "externalptr" classes to
#' reference attributes that have been looked up. A relevant scenario is writing
#' functions with a C++ implementation that are compiled and linked using
#' `Rcpp::cppFunction()` or `Rcpp::sourceCpp()`
#' @name cl_rework
#' @rdname cl_rework
#' @examples
#' \donttest{
#' library(Rcpp)
#' 
#' cppFunction(
#'   'Rcpp::StringVector get_str(
#'      SEXP corpus,
#'      SEXP p_attribute,
#'      SEXP registry,
#'      Rcpp::IntegerVector cpos
#'    ){
#'      SEXP attr;
#'      Rcpp::StringVector result;
#'      attr = RcppCWB::p_attr(corpus, p_attribute, registry);
#'      result = RcppCWB::cpos_to_str(attr, cpos);
#'      return(result);
#'   }',
#'   depends = "RcppCWB"
#' )
#'
#' result <- get_str("REUTERS", "word", RcppCWB::get_tmp_registry(), 0:50)
#' }
NULL


