library(Rcpp)
library(ModularTMBExample)


m <- Rcpp::Module(module = "growth",PACKAGE = "ModularTMBExample")

str(m)
c <- new(m$vonBertalanffy, 10)
c$check_list()
c$log_l_inf[[1]]$value <- 10
c$check_list()

