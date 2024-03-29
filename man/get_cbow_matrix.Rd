% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/cbow.R
\name{get_cbow_matrix}
\alias{get_cbow_matrix}
\title{Get CBOW Matrix.}
\usage{
get_cbow_matrix(
  corpus,
  p_attribute,
  registry = Sys.getenv("CORPUS_REGISTRY"),
  matrix,
  window
)
}
\arguments{
\item{corpus}{a CWB corpus}

\item{p_attribute}{a positional attribute}

\item{registry}{the registry directory}

\item{matrix}{a matrix}

\item{window}{window size}
}
\description{
Get matrix with moving windows. Negative integer values indicate absence of a
token at the respective position.
}
\examples{
m <- get_region_matrix(
  corpus = "REUTERS", s_attribute = "places",
  strucs = 0L:5L, registry = get_tmp_registry()
  )
windowsize <- 3L
m2 <- get_cbow_matrix(
  corpus = "REUTERS", p_attribute = "word",
  registry = get_tmp_registry(), matrix = m, window = windowsize
  )
colnames(m2) <- c(-windowsize:-1, "node", 1:windowsize)
}
