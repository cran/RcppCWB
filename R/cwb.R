#' CWB Tools for Creating Corpora
#' 
#' Wrappers for the CWB tools (\code{cwb-makeall}, \code{cwb-huffcode},
#' \code{cwb-compress-rdx}). Unlike the 'original' command line tools, these
#' wrappers will always perform a specific indexing/compression step on one
#' positional attribute, and produce all components.
#' 
#' @rdname cwb_utils
#' @param corpus name of a CWB corpus (upper case)
#' @param p_attribute name p-attribute
#' @param registry path to the registry directory, defaults to the value of the
#'   environment variable CORPUS_REGISTRY
#' @examples
#' # The package includes and 'unfinished' corpus of debates in the UN General 
#' # Assembly ("UNGA"), i.e. it does not yet include the reverse index, and it is
#' # not compressed.
#' #
#' # The first step in the following example is to copy the raw
#' # corpus to a temporary place.
#' 
#' registry <- if (!check_pkg_registry_files()) use_tmp_registry() else get_pkg_registry()
#' home_dir <- system.file(package = "RcppCWB", "extdata", "cwb", "indexed_corpora", "unga")
#' 
#' tmpdir <- tempdir()
#' win <- if (Sys.info()[["sysname"]] == "Windows") TRUE else FALSE
#' if (win) tmpdir <- normalizePath(tmpdir)
#' tmp_regdir <- file.path(tmpdir, "registry", fsep = if (win) "\\" else "/")
#' tmp_data_dir <- file.path(tmpdir, "indexed_corpora", fsep = if (win) "\\" else "/")
#' tmp_unga_dir <- file.path(tmp_data_dir, "unga", fsep = if (win) "\\" else "/")
#' if (!file.exists(tmp_regdir)) dir.create(tmp_regdir)
#' if (!file.exists(tmp_data_dir)) dir.create(tmp_data_dir)
#' if (!file.exists(tmp_unga_dir)){
#'   dir.create(tmp_unga_dir)
#' } else {
#'   file.remove(list.files(tmp_unga_dir, full.names = TRUE))
#' }
#' regfile <- readLines(file.path(registry, "unga"))
#' homedir_line <- grep("^HOME", regfile)
#' regfile[homedir_line] <- sprintf('HOME "%s"', tmp_unga_dir)
#' writeLines(text = regfile, con = file.path(tmp_regdir, "unga"))
#' for (x in list.files(home_dir, full.names = TRUE)){
#'   file.copy(from = x, to = tmp_unga_dir)
#' }
#' 
#' # perform cwb_makeall (equivalent to cwb-makeall command line utility)
#' cwb_makeall(corpus = "UNGA", p_attribute = "word", registry = tmp_regdir)
#' 
#' # see whether it works
#' ids_sentence_1 <- cl_cpos2id(
#'   corpus = "UNGA", p_attribute = "word", registry = tmp_regdir,
#'   cpos = 0:83
#'   )
#' tokens_sentence_1 <- cl_id2str(
#'   corpus = "UNGA", p_attribute = "word",
#'   registry = tmp_regdir, id = ids_sentence_1
#'   )
#' sentence <- gsub("\\s+([\\.,])", "\\1", paste(tokens_sentence_1, collapse = " "))
#' @rdname cwb_utils
#' @export cwb_makeall
cwb_makeall <- function(corpus, p_attribute, registry = Sys.getenv("CORPUS_REGISTRY")){
  .cwb_makeall(x = corpus, p_attribute = p_attribute, registry_dir = registry)
}


#' @rdname cwb_utils
#' @export cwb_huffcode
#' @examples 
#' cwb_huffcode(corpus = "UNGA", p_attribute = "word", registry = tmp_regdir)
cwb_huffcode <- function(corpus, p_attribute, registry = Sys.getenv("CORPUS_REGISTRY")){
  .cwb_huffcode(x = corpus, p_attribute = p_attribute, registry_dir = registry)
}

#' @rdname cwb_utils
#' @export cwb_compress_rdx
#' @examples 
#' cwb_compress_rdx(corpus = "UNGA", p_attribute = "word", registry = tmp_regdir)
cwb_compress_rdx <- function(corpus, p_attribute, registry = Sys.getenv("CORPUS_REGISTRY")){
  .cwb_compress_rdx(x = corpus, p_attribute = p_attribute, registry_dir = registry)
}