# A simple example showing how to use portable models
# with Rcpp and TMB

## devtools::install()
library(TMB)
##library(Rcpp)
##library(ModularTMBExample)
library(dplyr)
##library(ggplot2)
##library(tmbstan)
##library(shinystan)
#theme_set(theme_bw())
devtools::load_all("C:/Users/chris/noaa-git/ModularTMBExample")
## simulate data, repeated measures for each fish so can use
## random effects
set.seed(2342)
source("R/simulate_data.R")
obs <- simulate_data()
#g <- ggplot(obs, aes(age, length)) + geom_line() +
#  geom_point(mapping=aes(y=obs), col=2) +
#  facet_wrap('fish')



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
nfish = 10
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


### Fit a series of models to it to prove that we can

### Have no random effects (turn off sigmas and RE vectors)
vonB$prepare()
(parameters <- list(p = m$get_parameter_vector(), r = m$get_random_effects_vector()))
obj <- MakeADFun(data=list(), parameters,
                 DLL="ModularTMBExample", silent=TRUE)
obj$fn()
str(obj$report())
obs$pred0 <- obj$report()$pred
m$get_parameter_list() 

#vonB$finalize()
#vonB$show()
## optimize
opt <- with(obj, nlminb(par, fn, gr))
obs$pred <- obj$report()$pred
 sdreport(obj)
g+ geom_line(data=obs, mapping=aes(y=pred), col=4)
## fit <- tmbstan(obj, init='last.par.best', chains=1)
## launch_shinystan(fit)


### make sure to recreate vonB from scratch above if you ran the
### code chunk above!!!

## Penalized ML: turn on RE vectors but leave sigmas off at the
## truth and treat the RE as FE
m$clear()
vonB$log_l_inf_is_estimated <- TRUE
vonB$log_k_is_estimated <- TRUE
vonB$prepare()
(parameters <- list(p = m$get_parameter_vector(), r=m$get_random_effects_vector()))
obj <- MakeADFun(data=list(), parameters, DLL="ModularTMBExample", silent=TRUE)
opt <- with(obj, nlminb(par, fn, gr))
obs$pred <- obj$report()$pred
g+ geom_line(data=obs, mapping=aes(y=pred), col=4)

## Turn on marginal ML estimation of Linf vector (but not k)
m$clear()
vonB$log_l_inf_is_random_effect <- TRUE
vonB$log_l_inf_sigma$estimable <- TRUE
vonB$prepare()
(parameters <- list(p = m$get_parameter_vector(), r=m$get_random_effects_vector()))
obj <- MakeADFun(data=list(), parameters, DLL="ModularTMBExample", silent=TRUE, random='r')
opt <- with(obj, nlminb(par, fn, gr))
obs$pred <- obj$report()$pred
g+ geom_line(data=obs, mapping=aes(y=pred), col=4)
sdreport(obj)

## Full RE estimation of both vectors
m$clear()
vonB$log_k_is_random_effect <- TRUE
vonB$log_k_sigma$estimable <- TRUE
vonB$prepare()
(parameters <- list(p = m$get_parameter_vector(), r=m$get_random_effects_vector()))
obj <- MakeADFun(data=list(), parameters, DLL="ModularTMBExample", silent=TRUE, random='r')
opt <- with(obj, nlminb(par, fn, gr))
obs$pred <- obj$report()$pred
g+ geom_line(data=obs, mapping=aes(y=pred), col=4)
sdreport(obj)

## Estimate blocks. Collapse vectors into two arbitrary sets of
## fixed effects. The vectors alternate which is easier to see if
## I initialize them differently
m$clear()
for(i in 1:nfish) vonB$log_l_inf[i] <- .01
vonB$log_k_is_random_effect <- FALSE
vonB$log_l_inf_is_random_effect <- FALSE
vonB$log_k_sigma$estimable <- FALSE
vonB$log_l_inf_sigma$estimable <- FALSE
vonB$prepare()
(parameters <- list(p = m$get_parameter_vector(), r=m$get_random_effects_vector()))
## Set the first and last 10 fish to be the same
x <- 1:22
x[c(1,3,5,7,9)] <- 50                   # Linf block 1
x[10+c(1,3,5,7,9)] <- 51                # Linf block 2
x[1+c(1,3,5,7,9)] <- 52                 # k block 1
x[11+c(1,3,5,7,9)] <- 53                # k block 2
map <- list(p=factor(x))
obj <- MakeADFun(data=list(), parameters, DLL="ModularTMBExample", silent=TRUE, map=map)
opt <- with(obj, nlminb(par, fn, gr))
obs$pred <- obj$report()$pred
g+ geom_line(data=obs, mapping=aes(y=pred), col=4)
ggplot(obs, aes(age, pred, color=factor(fish))) + geom_line()
obj$report()
sdreport(obj)


## Map off parts of a vector. E.g. turn off estimation for some fish
m$clear()
for(i in 1:nfish) vonB$log_l_inf[i] <- 0
vonB$prepare()
(parameters <- list(p = m$get_parameter_vector(), r=m$get_random_effects_vector()))
## Same as above but turn off estimation of Linf for first 5 fish
x <- 1:22
x[c(1,3,5,7,9)] <- 50                   # Linf block 1
x[10+c(1,3,5)] <- 51                    # Linf block 2
x[10+c(7,9)] <- NA                      # Linf off
x[1+c(1,3,5,7,9)] <- 52                 # k block 1
x[11+c(1,3,5,7,9)] <- 53                # k block 2
map <- list(p=factor(x))
obj <- MakeADFun(data=list(), parameters, DLL="ModularTMBExample", silent=TRUE, map=map)
opt <- with(obj, nlminb(par, fn, gr))
obs$pred <- obj$report()$pred
g+ geom_line(data=obs, mapping=aes(y=pred), col=4)
ggplot(obs, aes(age, pred, color=factor(fish))) + geom_line()
obj$report()$linf |> unique() # not working??
sdreport(obj)


## Likelihood profiling
opt$par
prof <- tmbprofile(obj, name=1)
plot(prof)

## Full integration with NUTS via tmbstan. Try it on the full
## integration one
m$clear()
vonB$log_k_is_random_effect <- TRUE
vonB$log_k_sigma$estimable <- TRUE
vonB$log_l_inf_is_random_effect <- TRUE
vonB$log_l_inf_sigma$estimable <- TRUE
vonB$prepare()
(parameters <- list(p = m$get_parameter_vector(), r=m$get_random_effects_vector()))
obj <- MakeADFun(data=list(), parameters, DLL="ModularTMBExample", silent=TRUE, random='r')
opt <- with(obj, nlminb(par, fn, gr))
library(tmbstan)
fit <- tmbstan(obj, iter=1500, chains=3, init='last.par.best')
print(fit)
post <- as.data.frame(fit)
out <- list()
tmp <- obs[,1:4]
for(i in 1:nrow(post)) out[[i]] <- cbind(tmp, rep=i, pred=obj$report(post[i,-ncol(post)])$pred)
pred <- bind_rows(out)
ggplot(pred, aes(age, pred, group=factor(rep), color=factor(fish))) + geom_point()
pairs(fit, pars=names(fit)[c(1:4, 19:21)])

