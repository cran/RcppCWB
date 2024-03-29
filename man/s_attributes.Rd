% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/cl.R, R/RcppExports.R
\name{CL: s_attributes}
\alias{CL: s_attributes}
\alias{cl_cpos2struc}
\alias{cl_struc2cpos}
\alias{cl_struc2str}
\alias{cl_cpos2lbound}
\alias{cl_cpos2rbound}
\title{Using Structural Attributes.}
\usage{
cl_cpos2struc(
  corpus,
  s_attribute,
  cpos,
  registry = Sys.getenv("CORPUS_REGISTRY")
)

cl_struc2cpos(
  corpus,
  s_attribute,
  registry = Sys.getenv("CORPUS_REGISTRY"),
  struc
)

cl_struc2str(
  corpus,
  s_attribute,
  struc,
  registry = Sys.getenv("CORPUS_REGISTRY")
)

cl_cpos2lbound(corpus, s_attribute, cpos, registry = NULL)

cl_cpos2rbound(corpus, s_attribute, cpos, registry = NULL)
}
\arguments{
\item{corpus}{name of a CWB corpus (upper case)}

\item{s_attribute}{name of structural attribute (character vector)}

\item{cpos}{An \code{integer} vector with corpus positions.}

\item{registry}{path to the registry directory, defaults to the value of the
environment variable CORPUS_REGISTRY}

\item{struc}{a struc identifying a region}
}
\description{
Structural attributes store the metadata of texts in a CWB
corpus and/or any kind of annotation of a region of text. The fundamental
unit are so-called strucs, i.e. indices of regions identified by a left and
a right corpus position. The corpus library (CL) offers a set of functions
to make the translations between corpus positions (cpos) and strucs
(struc).
}
\details{
\code{cl_cpos2rbound()} and \code{cl_cpos2lbound()} return \code{NA} for values of
cpos that are outside a struc for the structural attribute given.
}
\examples{
# get metadata for matches of token
# scenario: id of the texts with occurrence of 'oil'
token_to_get <- "oil"
token_id <- cl_str2id("REUTERS", p_attribute = "word", str = "oil", get_tmp_registry())
token_cpos <- cl_id2cpos("REUTERS", p_attribute = "word", id = token_id, get_tmp_registry())
strucs <- cl_cpos2struc("REUTERS", s_attribute = "id", cpos = token_cpos, get_tmp_registry())
strucs_unique <- unique(strucs)
text_ids <- cl_struc2str("REUTERS", s_attribute = "id", struc = strucs_unique, get_tmp_registry())

# get the full text of the first text with match for 'oil'
left_cpos <- cl_cpos2lbound(
  "REUTERS", s_attribute = "id",
  cpos = min(token_cpos),
  registry = get_tmp_registry()
)
right_cpos <- cl_cpos2rbound(
  "REUTERS",
  s_attribute = "id",
  cpos = min(token_cpos),
  registry = get_tmp_registry()
)
txt <- cl_cpos2str(
  "REUTERS", p_attribute = "word",
  cpos = left_cpos:right_cpos,
  registry = get_tmp_registry()
)
fulltext <- paste(txt, collapse = " ")

# alternativ approach to achieve same result
first_struc_match_oil <- cl_cpos2struc(
  "REUTERS", s_attribute = "id",
  cpos = min(token_cpos),
  registry = get_tmp_registry()
  )
cpos_struc <- cl_struc2cpos(
  "REUTERS", s_attribute = "id",
  struc = first_struc_match_oil,
  registry = get_tmp_registry()
)
txt <- cl_cpos2str(
  "REUTERS",
  p_attribute = "word",
  cpos = cpos_struc[1]:cpos_struc[2],
  registry = get_tmp_registry()
)
fulltext <- paste(txt, collapse = " ")
}
