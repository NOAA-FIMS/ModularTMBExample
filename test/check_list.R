library(Rcpp)
library(ModularTMBExample)


m <- Rcpp::Module(module = "growth",PACKAGE = "ModularTMBExample")

str(m)
c <- new(m$vonBertalanffy, 10)
c$check_list()
c$l[[1]]$value <- 10
c$check_list()

