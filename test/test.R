# A simple example showing how to use portable models
# with Rcpp and TMB

f(is.loaded(dynlib('ModularTMBExample'))) dyn.unload(dynlib('ModularTMBExample'))
devtools::install()

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

for(i in 1:nfish) vonB$log_k$value[i] <- .0000123

vonB$log_l_inf_mean$value <-log(10)
vonB$log_l_inf_mean$estimable<- TRUE

vonB$log_l_inf_sigma$value <- log(.1)
vonB$log_l_inf_sigma$estimable <- FALSE

for(i in 1:nfish) vonB$k[i] <- exp(-0.00006)
vonB$log_k$log_k_is_estimated <- TRUE
vonB$log_k$log_k_is_random_effect <- TRUE

vonB$log_l_inf$log_l_inf_is_estimated <- TRUE
vonB$log_l_inf$log_l_inf_is_random_effect<- TRUE

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
str(obj$report(obj$par))
obs$pred0 <- obj$report(obj$par)$pred
vonB$finalize(obj$par)
vonB$show()
## optimize
opt <- with(obj, nlminb(par, fn, gr))
vonB$finalize(opt$par)
vonB$show()
obs$pred <- obj$report(opt$par)$pred
g <- ggplot(obs, aes(age, length)) + geom_line() +
  geom_point(mapping=aes(y=obs), col=2) +
  geom_line(mapping=aes(y=pred0), col=3) +
  geom_line(mapping=aes(y=pred), col=4) +
  facet_wrap('fish')
g


## Penalized ML: turn on RE vectors but leave sigmas off at the
## truth
vonB$log_l_inf$estimable <- rep(TRUE, nfish)
vonB$log_k$estimable <- rep(TRUE, nfish)
vonB$prepare()
parameters <- list(p = gg$get_parameter_vector())
obj <- MakeADFun(data=list(), parameters, DLL="ModularTMBExample")
obj$fn()
opt <- with(obj, nlminb(par, fn, gr))
vonB$finalize(opt$par)
vonB$show()
obs$pred <- obj$report(opt$par)$pred
g <- ggplot(obs, aes(age, length)) + geom_line() +
  geom_point(mapping=aes(y=obs), col=2) +
  geom_line(mapping=aes(y=pred0), col=3) +
  geom_line(mapping=aes(y=pred), col=4) +
  facet_wrap('fish')
g



## Turn on marginal ML estimation of Linf vector (but not k)
vonB$log_l_inf_sigma$estimable <- TRUE

## Full RE estimation of both vectors
vonB$log_k_sigma$estimable <- TRUE

## Likelihood profiling (via TMB?)

## Full integration with NUTS via tmbstan
library(tmbstan)
fit <- tmbstan(obj)
print(fit)

## Variational inference via tmbstan




obj$gr()
obj$fn()
random <-  which(!1:length(obs$obs) %in% 1:4)
obj$env$.random <- random
obj$par <- obj$par[-random]
obj$retape()
## Check it worked
obj$env$last.par.best
obj$fn()
obj$gr()

## Fit model
system.time(opt <- nlminb(obj$par, obj$fn, obj$gr))
rep <- sdreport(obj)

#update the von Bertalanffy object with updated parameters
vonB$finalize(opt$par)

#show results
vonB$show()

#show final gradient
print("final gradient:")
print(rep$gradient.fixed)

obs$pred <- obj$report()$pred
g <- ggplot(obs, aes(age, length)) + geom_line() +
  geom_point(mapping=aes(y=obs), col=2) +
  geom_line(mapping=aes(y=pred), col=3) +
  facet_wrap('fish')
g





runExample('simple')
obj2 <- MakeADFun(data=list(x=x, B=B, A=A),
                 parameters=list(u=u*0, beta=beta*0, logsdu=1, logsd0=1),
                 DLL="simple", silent=TRUE)
## update RE and try to match the standard way
obj2$env$.random <- obj$env$random
obj2$par <- obj$par
obj2$retape()
opt2 <- nlminb(obj2$par, obj2$fn, obj2$gr)
opt2$par-opt$par # yes it works
