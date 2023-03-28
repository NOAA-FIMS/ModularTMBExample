

## devtools::install()
library(TMB)
library(Rcpp)
library(ModularTMBExample)
library(dplyr)
library(ggplot2)
library(tmbstan)


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
g <- ggplot(obs, aes(age, length)) + geom_line() +
  geom_point(mapping=aes(y=obs), col=2) +
  facet_wrap('fish')



#get the Rcpp module
m <- Rcpp::Module(module = "growth",PACKAGE = "ModularTMBExample")
data <- list(obs=obs$obs, fish=obs$fish, age=obs$age)
#clear the parameter list, if there already is one
m$clear();
#create a von Bertalanffy object
vonB <- new(m$vonBertalanffy)
## Global settings for all cases below
#initialize k
vonB$log_k_mean$value <- log(.5)
vonB$log_k_mean$estimable <- TRUE
vonB$log_k_sigma$value <- log(.1)

vonB$log_k<- new(m$VariableVector, nfish)
for(i in 1:nfish) vonB$log_k$set(i-1,0.01)
## initialize linf
vonB$log_l_inf_mean$value <-log(10)
vonB$log_l_inf_mean$estimable<- TRUE
vonB$log_l_inf_sigma$value <- log(.1)

vonB$log_l_inf<- new(m$VariableVector, nfish)
for(i in 1:nfish) vonB$log_l_inf$set(i-1,0.01)
vonB$a_min$value <- .001
vonB$a_min$estimable <- FALSE
#set data
vonB$obs <-data$obs
vonB$nfish <- nfish
vonB$fish <- data$fish-1
vonB$ages<- data$age
vonB$predicted <- rep(0,len=nrow(obs))

## ones we change below, start w/ just two fixed effects (means)
vonB$log_k_sigma$estimable <- FALSE
vonB$log_k_is_estimated <- TRUE
vonB$log_k_is_random_effect <- FALSE

vonB$log_l_inf_sigma$estimable <- FALSE
vonB$log_l_inf_is_estimated <- TRUE
vonB$log_l_inf_is_random_effect<- FALSE



### Fit a series of models to it to prove that we can

### Have no random effects (turn off sigmas and RE vectors)
vonB$prepare()

(parameters <- list(p = m$get_parameter_vector(), r = m$get_random_effects_vector()))

m$get_parameter_vector()
m$get_random_effects_vector()
obj <- MakeADFun(data=list(), parameters,
                 DLL="ModularTMBExample", silent=FALSE)
obj$fn()


system.time(opt <- nlminb(obj$par, obj$fn, obj$gr))
#obs$pred <- obj$report()$pred
#obj$report()

for(i in 0:(nfish-1)){
    print(vonB$log_k$at(i)$random_parameter_index)
}

for(i in 0:(nfish-1)){
    print(vonB$log_l_inf$at(i)$random_parameter_index)
}

m$get_random_effects_vector()

m$get_parameter_vector()
