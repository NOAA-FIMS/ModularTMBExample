# A simple example showing how to use portable models
# with Rcpp and TMB

library(TMB)
library(Rcpp)
library(ModularTMBExample)

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
Dat <- new(g$ObsData) 
Dat$Data <- data

#set ages 
Dat$ages<-ages

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
