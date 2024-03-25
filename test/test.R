# A simple example showing how to use portable models
# with Rcpp and TMB

library(TMB)
library(Rcpp)
#library(ModularTMBExample)
devtools::load_all()

#get the Rcpp module
g<-Rcpp::Module(module = "growth",PACKAGE = "ModularTMBExample")

#simulate data
l_inf<- 10.0
a_min<- 0.1
k<- .5
ages<-c(0.1, 1,2,3,4,5,6,7,8)
#data<-c(replicate(length(ages), 0.0), 0.0)
data<-replicate(length(ages), 0.0)

for(i in 1:length(ages)){
  data[i] = (l_inf * (1.0 - exp(-k * (ages[i] - a_min))))* runif(1,.90,1.5)
}

#clear the parameter list, if there already is one
g$clear();

#create a von Bertalanffy object
vonB<-new(g$vonBertalanffy)

#initialize k
vonB$k$value<-.05
vonB$k$estimable<-TRUE

#initialize a_min
vonB$a_min$value<-.01
vonB$a_min$estimable<-FALSE

#initialize l_inf
vonB$l_inf$value<-7
vonB$l_inf$estimable<-TRUE

#set data
Pop <- new(g$Population) 
#set ages 
Pop$ages<-ages

DataNLL <- new(g$NormalNLL)
DataNLL$x <- data
DataNLL$log_sd <- 0
DataNLL$estimate_log_sd <- TRUE
DataNLL$SetMu(Pop$get_id(), "growth")

#Set up prior on K based on fish life value - Normal example
# this was a long install with a lot of dependencies, pulling out parameter values to avoid this for testing
# remotes::install_github("James-Thorson-NOAA/FishLife")

# library(FishLife)
# library(mvtnorm)
# params <- matrix(c('Loo', 'K'), ncol=2)
# x <- Search_species(Genus="Hippoglossoides")$match_taxonomy
# y <- Plot_taxa(x, params=params)

## multivariate normal in log space for two growth parameters
mu <- c(Loo = 3.848605, K = -1.984452) #y[[1]]$Mean_pred[params]
Sigma <- rbind(c( 0.1545170, -0.1147763),
               c( -0.1147763,  0.1579867)) #y[[1]]$Cov_pred[params, params]
row.names(Sigma) <- c('Loo', 'K')
colnames(Sigma) <- c('Loo', 'K')

GrowthKPrior <- new(g$NormalNLL)
GrowthKPrior$mu <- mu[2]
GrowthKPrior$log_sd <- log(Sigma[2,2])
GrowthKPrior$SetX(vonB$get_id(), "k")

#prepare for interfacing with TMB
g$CreateModel()

#create an empty data list (data set above)
data <- list()

#create a parameter list
parameters <- list(
  p = g$get_parameter_vector()
)

obj <- MakeADFun(data, parameters, DLL="ModularTMBExample")
newtonOption(obj, smartsearch=FALSE)

print(obj$gr(obj$par))

## Fit model
opt <- nlminb(obj$par, obj$fn, obj$gr)
rep <- sdreport(obj)

rep

#update the von Bertalanffy object with updated parameters
vonB$finalize(rep$par.fixed)

#show results
vonB$show()

obj$report()

#show final gradient
print("final gradient:")
print(rep$gradient.fixed)
