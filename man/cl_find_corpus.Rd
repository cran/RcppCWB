% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/cl.R
\name{cl_find_corpus}
\alias{cl_find_corpus}
\title{Load corpus.}
\usage{
cl_find_corpus(corpus, registry)
}
\arguments{
\item{corpus}{name of a CWB corpus (upper case)}

\item{registry}{path to the registry directory, defaults to the value of the
environment variable CORPUS_REGISTRY}
}
\value{
A \code{externalptr} referencing the C representation of the corpus.
}
\description{
Load corpus.
}
