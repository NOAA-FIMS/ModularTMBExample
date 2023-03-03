library(TMB)
library(Rcpp)
library(ModularTMBExample)
library(dplyr)
library(ggplot2)
library(tmbstan)
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
vonB <- new(m$vonBertalanffy, nfish)
vonB$check_list()
## Global settings for all cases below
#initialize k
vonB$log_k_mean$value <- log(.5)
vonB$log_k_mean$estimable <- TRUE
vonB$log_k_sigma$value <- log(.1)
for(i in 1:nfish) {
    print(i)
    vonB$log_l_inf[[i]]$value<-0
    
}
vonB$check_list()
#traceback()
#q()

## initialize linf
vonB$log_l_inf_mean$value <-log(10)
vonB$log_l_inf_mean$estimable<- TRUE
vonB$log_l_inf_sigma$value <- log(.1)


#for(i in 1:nfish) vonB$log_l_inf[[i]]$value<-0.01
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



vonB$log_l_inf[[1]]$value<-10
vonB$log_l_inf[[1]]$estimable<-TRUE
vonB$log_l_inf[[1]]$is_random_effect<-TRUE#$is_random_effect<-TRUE
vonB$log_l_inf[[6]]$value<-0.1
vonB$log_l_inf[[6]]$estimable<-TRUE#$estimable<-FALSE
vonB$log_l_inf[[6]]$is_random_effect<-TRUE#$is_random_effect<-TRUE

vonB$log_k[[1]]$value<-0.01
vonB$log_k[[1]]$estimable<-TRUE#$estimable<-FALSE
vonB$log_k[[1]]$is_random_effect<-TRUE#$is_random_effect<-TRUE
vonB$log_k[[6]]$value<-0.01
vonB$log_k[[6]]$estimable<-TRUE#$estimable<-FALSE
vonB$log_k[[6]]$is_random_effect<-TRUE#$is_random_effect<-TRUE
#set up map
for(i in 2:5){
    vonB$log_l_inf[[i]]$estimable<-FALSE
    m$map_to(vonB$log_l_inf[[i]], vonB$log_l_inf[[1]])
    
    vonB$log_k[[i]]$estimable<-FALSE
    m$map_to(vonB$log_k[[i]], vonB$log_k[[1]])
}

for(i in 7:nfish){
    vonB$log_l_inf[[i]]$estimable<-FALSE
    m$map_to(vonB$log_l_inf[[i]], vonB$log_l_inf[[6]])
    
    vonB$log_k[[i]]$estimable<-FALSE
    m$map_to(vonB$log_k[[i]], vonB$log_k[[6]])
}



vonB$prepare()
(parameters <- list(p = m$get_parameter_vector(), r=m$get_random_effects_vector()))
obj <- MakeADFun(data=list(), parameters,random = 'r', DLL="ModularTMBExample", silent=FALSE)
opt <- with(obj, nlminb(par, fn, gr))
obs$pred <- obj$report()$pred
g+ geom_line(data=obs, mapping=aes(y=pred), col=4)
obj$report()

vonB$finalize()
vonB$show()
m$get_random_effects_vector()
