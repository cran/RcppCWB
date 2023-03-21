## -----------------------------------------------------------------------------
library(Rcpp)
library(RcppCWB)

## -----------------------------------------------------------------------------
p_attr_word <- p_attr(
  corpus = "REUTERS",
  p_attribute = "word",
  registry = get_tmp_registry()
)

## -----------------------------------------------------------------------------
cpos_to_str(p_attr_word, 0:10)

## -----------------------------------------------------------------------------
cppFunction(
  'Rcpp::StringVector get_str(SEXP corpus, SEXP p_attribute, SEXP registry, Rcpp::IntegerVector cpos){
     SEXP attr;
     Rcpp::StringVector result;
     attr = RcppCWB::p_attr(corpus, p_attribute, registry);
     result = RcppCWB::cpos_to_str(attr, cpos);
     return(result);
  }',
  depends = "RcppCWB"
)

## -----------------------------------------------------------------------------
get_str("REUTERS", "word", RcppCWB::get_tmp_registry(), 0:50)

## -----------------------------------------------------------------------------
sourceCpp(file = system.file(package = "RcppCWB", "cpp", "fastdecode.cpp"))

## -----------------------------------------------------------------------------
outfile <- tempfile(fileext = ".txt")

write_token_stream(
  corpus = "REUTERS",
  p_attribute = "word", 
  s_attribute = "id",
  attribute_type = "s",
  registry = RcppCWB::get_tmp_registry(),
  filename = outfile
)

## -----------------------------------------------------------------------------
readLines(outfile) |>
  lapply(substr, 1, 75) |>
  unlist()

