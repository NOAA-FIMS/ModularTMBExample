# A simple example showing how to use portable models
# with Rcpp and TMB

library(TMB)
library(Rcpp)
library(ModularTMBExample)
library(dplyr)
library(ggplot2)


## simulate data, repeated measures for each fish so can use
## random effects

set.seed(2342)
nfish <- 10
ages <- c(0.1, 1:15)
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
  geom_point(mapping=aes(y=obs), col=2) + facet_wrap('fish')
## g


#get the Rcpp module
g<-Rcpp::Module(module = "growth",PACKAGE = "ModularTMBExample")
data <- list(obs=obs$obs, fish=obs$fish, age=obs$age)
#clear the parameter list, if there already is one
g$clear();
#create a von Bertalanffy object
vonB<-new(g$vonBertalanffy)
#initialize k
vonB$log_k_mean$value<-log(.05)
vonB$log_k_mean$estimable<-TRUE
vonB$log_k_sigma$value <- log(1)
vonB$log_k_sigma$estimable<-TRUE
for(i in 1:nfish) vonB$log_k$value[i] <- 0
vonB$log_l_inf_mean$value<-log(20)
vonB$log_l_inf_mean$estimable<-TRUE
vonB$log_l_inf_sigma$value <- log(1)
vonB$log_l_inf_sigma$estimable<-TRUE
for(i in 1:nfish) vonB$log_l_inf$value[i] <- 0
vonB$a_min$value<-0
vonB$a_min$estimable<-FALSE
#set data
vonB$obs <-data$obs
vonB$nfish <- nfish
vonB$fish <- data$fish-1
vonB$ages<- data$age
vonB$predicted <- rep(0,len=nrow(obs))
vonB$log_k$estimable <- rep(TRUE, nfish)
vonB$log_l_inf$estimable <- rep(TRUE, nfish)
str(vonB)

##prepare for interfacing with TMB
vonB$prepare()

#create an empty data list (data set above)
data <- list()

#create a parameter list
parameters <- list(
  p = g$get_parameter_vector()
)

vonB$evaluate()

#make the AD function in TMB
obj <- MakeADFun(data, parameters, DLL="ModularTMBExample")
obj$fn()
obj$gr()
newtonOption(obj, smartsearch=FALSE)

## Fit model
system.time(opt <- nlminb(obj$par, obj$fn, obj$gr))
rep <- sdreport(obj)

#update the von Bertalanffy object with updated parameters
vonB$finalize(rep$par.fixed)

#show results
vonB$show()

#show final gradient
print("final gradient:")
print(rep$gradient.fixed)

