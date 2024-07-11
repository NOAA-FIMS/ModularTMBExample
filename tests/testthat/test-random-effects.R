# A simple example showing how to use portable models
# with Rcpp and TMB


test_that("test single nll",{
#simulate data
l_inf<- 3
a_min<- 0.1
k<- .2
ages<-c(a_min, 1,2,3,4,5,6,7,8,9,10,11,12)
#data<-c(replicate(length(ages), 0.0), 0.0)
Length <- u <- replicate(length(ages), 0.0)

for(i in 1:length(ages)){
  Length[i] = (l_inf * (1.0 - exp(-k * (ages[i] - a_min))))
}
#simulate randomwalk on ages
set.seed(234)
u[1] + rnorm(1, 0, 0.25)
for(i in 2:length(Length)){
  u[i] <- rnorm(1, u[i-1], 0.25)
}
length.data <- Length + u

#clear the parameter list, if there already is one
clear()

#create a von Bertalanffy object
vonB<-new(vonBertalanffy)

#initialize k
vonB$logk$value<-log(k)
vonB$logk$estimable<-TRUE

#initialize a_min
vonB$a_min$value<-a_min
vonB$a_min$estimable<-FALSE

#initialize l_inf
vonB$l_inf$value<-max(length.data)
vonB$l_inf$estimable<-TRUE

#set data
Pop <- new(Population) 
#set ages 
Pop$ages<-ages
Pop$set_growth(vonB$get_id())

DataLL <- new(NormalLPDF)

DataLL$observed_value <- new(VariableVector, length.data, length(length.data))

DataLL$log_sd <- new(VariableVector, 1)
DataLL$log_sd[1]$value <- -1
DataLL$log_sd[1]$estimable <- TRUE
DataLL$input_type <- "data"
DataLL$simulate_flag <- TRUE 
paste0(Pop$get_module_name(), "_", Pop$get_id(), "_length")
DataLL$set_distribution_links("data", Pop$get_id(), Pop$get_module_name(), "length")


#prepare for interfacing with TMB
CreateModel()


#create a data list (data set above)
Data <- list(
  y = get_data_vector()
)

#create a parameter list
Parameters <- list(
  p = get_parameter_vector()
)

obj <- TMB::MakeADFun(Data, Parameters, DLL="ModularTMBExample", trace = TRUE)
#newtonOption(obj, smartsearch=FALSE)

print(obj$gr(obj$par))

## Fit model
opt <- nlminb(obj$par, obj$fn, obj$gr)
sdr <- TMB::sdreport(obj)
named.pars <- get_parameter_names(opt$par)

mean.sdr <- as.list(sdr, "Est")$p
std.sdr <- as.list(sdr, "Std")$p
ci <- list()
for(i in seq_along(mean.sdr)){
  ci[[i]] <- mean.sdr[i] + c(-1,1)*qnorm(.975)*std.sdr[i]
}

expect_equal( log(k) > ci[[1]][1] & log(k) < ci[[1]][2], TRUE)
expect_equal( l_inf > ci[[2]][1] & l_inf < ci[[2]][2], TRUE)
expect_equal( log(.1) > ci[[3]][1] & log(.1) < ci[[3]][2], TRUE)


#Test osa
osa.fg <- TMB::oneStepPredict(obj, observation.name = "y", method = "fullGaussian")
osa.osg <- TMB::oneStepPredict(obj, observation.name = "y", 
                               data.term.indicator = "keep", method = "oneStepGaussian")
osa.gen <- TMB::oneStepPredict(obj, observation.name = "y", 
                               data.term.indicator = "keep", method = "oneStepGeneric")
osa.cdf <- TMB::oneStepPredict(obj, observation.name = "y", 
                               data.term.indicator = "keep", method = "cdf")
exp.value <-  (opt$par[2] * (1.0 - exp(-exp(opt$par[1]) * (ages - a_min))))
pear.resid <- (length.data-exp.value)/ exp(opt$par[3])


expect_equal(pear.resid, osa.fg$residual)
expect_equal(pear.resid, osa.osg$residual)
expect_equal(pear.resid, osa.gen$residual, tolerance = 1e-4)
expect_equal(pear.resid, osa.cdf$residual)
ks.test(pear.resid, 'pnorm')$p.value
#model fails correlation test
DHARMa::testTemporalAutocorrelation(pear.resid, ages)$p.value

## test random effects ======================================
l_inf<- 3
a_min<- 0.1
k<- .2
ages<-c(a_min, 1,2,3,4,5,6,7,8,9,10,11,12)
nobs <- length(ages)
#data<-c(replicate(length(ages), 0.0), 0.0)
Length <- u <- replicate(length(ages), 0.0)

for(i in 1:length(ages)){
  Length[i] = (l_inf * (1.0 - exp(-k * (ages[i] - a_min))))
}
#simulate randomwalk on ages
set.seed(234)
u[1] + rnorm(1, 0, 0.25)
for(i in 2:length(Length)){
  u[i] <- rnorm(1, u[i-1], 0.25)
}
length.data <- Length + u
#fit random effect model
clear()

#create a von Bertalanffy object
vonB<-new(vonBertalanffy)

#initialize k
vonB$logk$value<-log(k)
vonB$logk$estimable<-TRUE

#initialize a_min
vonB$a_min$value<-a_min
vonB$a_min$estimable<-FALSE

#initialize l_inf
vonB$l_inf$value<-max(length.data)
vonB$l_inf$estimable<-TRUE

#set data
Pop <- new(Population) 
#set ages 
Pop$ages<-ages
Pop$set_growth(vonB$get_id())
init.re <- rnorm(nobs)
Pop$u <- new(VariableVector, init.re, length(length.data))
Pop$u$set_all_random_effects(TRUE) 

DataLL <- new(NormalLPDF)

DataLL$observed_value <- new(VariableVector, length.data, length(length.data))

DataLL$log_sd <- new(VariableVector, 1)
DataLL$log_sd[1]$value <- -1
DataLL$log_sd[1]$estimable <- TRUE
DataLL$input_type <- "data"
DataLL$simulate_flag <- TRUE 
DataLL$set_distribution_links("data", Pop$get_id(), Pop$get_module_name(), "length")

Ar1LL <- new(AR1LPDF)
Ar1LL$rho <- new(VariableVector, 1.0, 1)
Ar1LL$log_sd <- new(VariableVector, 1)
Ar1LL$log_sd[1]$value <- 0 
Ar1LL$log_sd[1]$estimable <- TRUE
Ar1LL$input_type <- "re"
Ar1LL$observed_value <- new(VariableVector, rep(0, nobs), nobs)

Ar1LL$set_distribution_links("re", Pop$get_id(), Pop$get_module_name(), "u")


#prepare for interfacing with TMB
CreateModel()


#create a data list (data set above)
Data <- list(
  y = get_data_vector()
)

#create a parameter list
Parameters <- list(
  p = get_parameter_vector(),
  re = get_random_effects_vector()
)



obj <- TMB::MakeADFun(data = Data, parameters = Parameters, random = "re",
                      DLL="ModularTMBExample", trace = TRUE)
#newtonOption(obj, smartsearch=FALSE)

print(obj$gr(obj$par))

## Fit model
opt <- nlminb(obj$par, obj$fn, obj$gr)

Ar1LL$finalize(obj$par)
Ar1LL$log_likelihood_vec

clear()

