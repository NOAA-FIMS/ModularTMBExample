gg<-Rcpp::Module("growth", PACKAGE="ModularTMBExample")

library(TMB)
library(Rcpp)
library(ModularTMBExample)
library(dplyr)
library(ggplot2)


## simulate data, repeated measures for each fish so can use
## random effects
set.seed(2342)
nfish <- 10
ages <- 1:15
nreps <- length(ages)

## parameters per fish
log_l_inf <- rnorm(nfish, log(10.0), sd=.1)
log_k <- rnorm(nfish, log(.5), sd=.1)
a_min <- 0.1
fish <- rep(1:nfish, each=nreps)
## simulate
obs <- lapply(1:nfish, function(i)
  data.frame(fish=i, age=ages,
             length=(exp(log_l_inf[i]))*(1.0-exp(-exp(log_k[i])*(ages-a_min))))) %>%
  bind_rows %>%
  mutate(obs=length*exp(rnorm(nfish*nreps,0-.05^2/2,.05)))



#get the Rcpp module
gg <- Rcpp::Module(module = "growth",PACKAGE = "ModularTMBExample")
data <- list(obs=obs$obs, fish=obs$fish, age=obs$age)
#clear the parameter list, if there already is one
gg$clear();
#create a von Bertalanffy object
vonB<-new(gg$vonBertalanffy)

#initialize k
vonB$log_k_mean$value<-log(.5)
vonB$log_k_mean$estimable<-TRUE
vonB$log_k_sigma$value <- log(.1)
vonB$log_k_sigma$estimable <- FALSE

for(i in 1:nfish) vonB$log_k[i] <- .0000123

vonB$log_l_inf_mean$value <-log(10)
vonB$log_l_inf_mean$estimable<- TRUE

vonB$log_l_inf_sigma$value <- log(.1)
vonB$log_l_inf_sigma$estimable <- FALSE

for(i in 1:nfish) vonB$log_l_inf[i] <- -0.00006
vonB$log_k_is_estimated <- TRUE
vonB$log_k_is_random_effect <- TRUE

vonB$log_l_inf_is_estimated <- TRUE
vonB$log_l_inf_is_random_effect<- TRUE

vonB$a_min$value <- .001
vonB$a_min$estimable <- FALSE
#set data
vonB$obs <-data$obs
vonB$nfish <- nfish
vonB$fish <- data$fish-1
vonB$ages<- data$age
vonB$predicted <- rep(0,len=nrow(obs))

### Have no random effects (turn off sigmas and RE vectors)
vonB$prepare()
parameters <- list(p = gg$get_parameter_vector(), r = gg$get_random_effects_vector())

obj <- MakeADFun(data=list(), parameters, random="r",  DLL="ModularTMBExample")


obj$fn()
#str(obj$report(obj$par))
#obs$pred0 <- obj$report(obj$par)$pred

## optimize
opt <- with(obj, nlminb(par, fn, gr))

vonB$finalize()
vonB$show()

