# A simple example showing how to use portable models
# with Rcpp and TMB

## devtools::install()
library(TMB)
library(Rcpp)
library(ModularTMBExample)
library(dplyr)
library(ggplot2)
library(tmbstan)
library(shinystan)
theme_set(theme_bw())

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
for(i in 1:nfish) vonB$log_k[i] <- 0
## initialize linf
vonB$log_l_inf_mean$value <-log(10)
vonB$log_l_inf_mean$estimable<- TRUE
vonB$log_l_inf_sigma$value <- log(.1)
for(i in 1:nfish) vonB$log_l_inf[i] <- 0
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
vonB$log_k_is_estimated <- FALSE
vonB$log_k_is_random_effect <- FALSE
vonB$log_l_inf_sigma$estimable <- FALSE
vonB$log_l_inf_is_estimated <- FALSE
vonB$log_l_inf_is_random_effect<- FALSE
str(vonB)

### Have no random effects (turn off sigmas and RE vectors)
vonB$prepare()
(parameters <- list(p = m$get_parameter_vector(), r = m$get_random_effects_vector()))
obj <- MakeADFun(data=list(), parameters,
                 DLL="ModularTMBExample", silent=TRUE)
obj$fn()
str(obj$report())
obs$pred0 <- obj$report()$pred
#vonB$finalize()
## vonB$show()
## optimize
opt <- with(obj, nlminb(par, fn, gr))
obs$pred <- obj$report()$pred
## sdreport(obj)
g+ geom_line(data=obs, mapping=aes(y=pred), col=4) +
  geom_line(data=obs, mapping=aes(y=pred0), col=5)
## fit <- tmbstan(obj, init='last.par.best', chains=1)
## launch_shinystan(fit)


### make sure to recreate vonB from scratch above if you ran the
### code chunk above!!!

## Penalized ML: turn on RE vectors but leave sigmas off at the
## truth and treat the RE as FE
vonB$log_l_inf_is_estimated <- TRUE
vonB$log_k_is_estimated <- TRUE
vonB$prepare()
(parameters <- list(p = m$get_parameter_vector(), r=m$get_random_effects_vector()))
map <- list(p=factor(c(rep(NA,2), 1:22)))
map <- list()
obj <- MakeADFun(data=list(), parameters, DLL="ModularTMBExample", silent=TRUE, map=map)
obs$pred0 <- obj$report()$pred
opt <- with(obj, nlminb(par, fn, gr))
obs$pred <- obj$report(opt$par)$pred
g+ geom_line(data=obs, mapping=aes(y=pred), col=4)

obj$report()
fit <- tmbstan(obj, init='last.par.best', chains=1)
launch_shinystan(fit)





### todo below here


## Turn on marginal ML estimation of Linf vector (but not k)
vonB$log_l_inf_sigma$estimable <- TRUE
parameters <- list(p = m$get_parameter_vector())

## Full RE estimation of both vectors
vonB$log_k_sigma$estimable <- TRUE
parameters <- list(p = m$get_parameter_vector())

## Estimate blocks. Collapse vectors into sets of fixed effects
vonB$log_k_sigma$estimable <- FALSE
vonB$log_l_inf_sigma$estimable <- FALSE

parameters <- list(p = m$get_parameter_vector())
obj <- MakeADFun(data=list(), parameters, DLL="ModularTMBExample")
opt <- with(obj, nlminb(par, fn, gr))
obs$pred <- obj$report(opt$par)$pred
sdreport(obj)
g+ geom_line(data=obs, mapping=aes(y=pred), col=4)

## Map off parts of a vector

## Likelihood profiling (via TMB?)

## Full integration with NUTS via tmbstan
library(tmbstan)
fit <- tmbstan(obj)
print(fit)

### Quick demo of the backdoor index approach to random effects
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
obj2$report()
obj$report()
sdreport(obj)
sdreport(obj2) # breaks!!

fit <- tmbstan(obj2) ## works!
