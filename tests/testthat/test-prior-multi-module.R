test_that("test multi-module prior",{

  
# #Get parameters from FishLife
# #install FishLife using: remotes::install_github("James-Thorson-NOAA/FishLife") 
# library(FishLife)
# params <- matrix(c('Loo', 'K'), ncol=2)
# x <- Search_species(Genus="Hippoglossoides")$match_taxonomy
# y <- Plot_taxa(x, params=params)

  
  # multivariate normal in log space for two growth parameters
  mu <- c(K = -1.984452, Linf = 3.848605) #y[[1]]$Mean_pred[params]
  Sigma <- rbind(c( 0.1579867, -0.1147763),
                 c( -0.1147763,  0.1545170)) #y[[1]]$Cov_pred[params, params]
  row.names(Sigma) <- c('K', 'Linf')
  colnames(Sigma) <- c('K', 'Linf')
  
  
  #simulate data
  set.seed(123)
  sim.parms <- mvtnorm::rmvnorm(2, mu, Sigma)
  l_inf<- sim.parms[,2]
  a_min<- 0.1
  k<- exp(sim.parms[,1])
  
  ages<-c(a_min, 1,2,3,4,5,6,7,8,9,10)
Length1 <- Length2 <- replicate(length(ages), 0.0)

for(i in 1:length(ages)){
  Length1[i] = (l_inf[1] * (1.0 - exp(-k[1] * (ages[i] - a_min))))
  Length2[i] = (l_inf[2] * (1.0 - exp(-k[2] * (ages[i] - a_min))))
}
#add observation error
set.seed(234)
length.data1 <- Length1 + rnorm(length(ages), 0, .1)
set.seed(345)
length.data2 <- Length2 + rnorm(length(ages), 0, .1)

#clear the parameter list, if there already is one
clear()

#create first von Bertalanffy object
vonB1<-new(vonBertalanffy)

#initialize logk
vonB1$logk$value<-log(.1)
vonB1$logk$estimable<-TRUE

#initialize a_min
vonB1$a_min$value<-.1
vonB1$a_min$estimable<-FALSE

#initialize l_inf
vonB1$l_inf$value<-max(length.data1)
vonB1$l_inf$estimable<-TRUE

#create second von Bertalanffy object
vonB2<-new(vonBertalanffy)

#initialize logk
vonB2$logk$value<-log(.1)
vonB2$logk$estimable<-TRUE

#initialize a_min
vonB2$a_min$value<-.1
vonB2$a_min$estimable<-FALSE

#initialize l_inf
vonB2$l_inf$value<-max(length.data2)
vonB2$l_inf$estimable<-TRUE

#setup first population, set ages and link to vonB1
Pop1 <- new(Population) 
#set ages 
Pop1$ages<-ages
Pop1$set_growth(vonB1$get_id())

#setup second population, set ages and link to vonB2
Pop2 <- new(Population) 
#set ages 
Pop2$ages<-ages
Pop2$set_growth(vonB2$get_id())

#setup data log-likelihood for Length1
DataLL1 <- new(NormalLPDF)
#input length.data1
DataLL1$observed_value <- 
  new(VariableVector, length.data1, length(length.data1))
#initialize log_sd
DataLL1$log_sd <- new(VariableVector, -2, 1)
DataLL1$log_sd[1]$value <- 0
DataLL1$log_sd[1]$estimable <- TRUE
DataLL1$input_type = "data"
#link data log-likelihood to length from Pop1
DataLL1$set_distribution_links("data", Pop1$get_id(), Pop1$get_module_name(), "length")

#setup data log-likelihood for Length2
DataLL2 <- new(NormalLPDF)
#input length.data2
DataLL2$observed_value <- 
  new(VariableVector, length.data2, length(length.data2))
#initialize log_sd
DataLL2$log_sd <- new(VariableVector, -2, 1)
DataLL2$log_sd[1]$value <- 0
DataLL2$log_sd[1]$estimable <- TRUE
DataLL2$input_type = "data"
#link data log-likelihood to length from Pop2
DataLL2$set_distribution_links("data", Pop2$get_id(), Pop2$get_module_name(), "length")

#set up shared multivariate prior for logk and l_inf
GrowthMVPrior <- new(MVNormLPDF)
GrowthMVPrior$expected_value <- new(VariableVector, mu, 2)
GrowthMVPrior$input_type <- "prior"
phi <- cov2cor(Sigma)[1,2]
GrowthMVPrior$log_sd <- new(VariableVector, 0.5*log(diag(Sigma)), 2)
GrowthMVPrior$logit_phi <- new(VariableVector, log((phi+1)/(1-phi)), 1)  
#link prior log-likelihood to the l_inf and logk parameters from vonB
GrowthMVPrior$set_distribution_links( "prior", 
                                      c(vonB1$get_id(), vonB1$get_id(),
                                        vonB2$get_id(), vonB2$get_id()),
                                      c(vonB1$get_module_name(),vonB1$get_module_name(),
                                        vonB2$get_module_name(),vonB2$get_module_name()),
                                      c("logk", "l_inf","logk", "l_inf"))

#prepare for interfacing with TMB
CreateModel()


#create a data list (data set above)
Data <- list(
  y = get_data_vector()
)

expect_equal(c(length.data1, length.data2), Data$y)

#create a parameter list
Parameters <- list(
  p = get_parameter_vector()
)

#setup TMB object
obj <- TMB::MakeADFun(Data, Parameters, DLL="ModularTMBExample")
#newtonOption(obj, smartsearch=FALSE)



print(obj$gr(obj$par))


## Fit model
opt <- nlminb(obj$par, obj$fn, obj$gr)
sdr <- TMB::sdreport(obj)

mean.sdr <- as.list(sdr, "Est")$p
std.sdr <- as.list(sdr, "Std")$p
ci <- list()
for(i in seq_along(mean.sdr)){
  ci[[i]] <- mean.sdr[i] + c(-1,1)*qnorm(.975)*std.sdr[i]
}

expect_equal( log(k[1]) > ci[[1]][1] & log(k[1]) < ci[[1]][2], TRUE)
expect_equal( l_inf[1] > ci[[2]][1] & l_inf[1] < ci[[2]][2], TRUE)
#expect_equal( log(k[2]) > ci[[3]][1] & log(k[2]) < ci[[3]][2], TRUE)
#expect_equal( l_inf[2] > ci[[4]][1] & l_inf[2] < ci[[4]][2], TRUE)
expect_equal( log(.1) > ci[[5]][1] & log(.1) < ci[[5]][2], TRUE)
#expect_equal( log(.1) > ci[[6]][1] & log(.1) < ci[[6]][2], TRUE)


fit <- tmbstan::tmbstan(obj, init = "best.last.par", iter = 4000)
#pairs(fit, pars=names(obj$par))
#traceplot(fit, pars=names(obj$par), inc_warmup=TRUE)
postmle <- as.matrix(fit)
bayes.pi <- rstantools::predictive_interval(postmle)

expect_equal(log(k[1]) > bayes.pi[1,1] & log(k[1]) < bayes.pi[1,2], TRUE)
expect_equal(l_inf[1] > bayes.pi[2,1] & l_inf[1] < bayes.pi[2,2], TRUE)
#expect_equal(log(k[2]) > bayes.pi[3,1] & log(k[2]) < bayes.pi[3,2], TRUE)
#expect_equal(l_inf[2] > bayes.pi[4,1] & l_inf[2] < bayes.pi[4,2], TRUE)
#expect_equal(log(.1) > bayes.pi[5,1] & log(.1) < bayes.pi[5,2], TRUE)
expect_equal(log(.1) > bayes.pi[6,1] & log(.1) < bayes.pi[6,2], TRUE)


clear()


# #update the von Bertalanffy object with updated parameters
# vonB$finalize(rep$par.fixed)

# #show results
# vonB$show()

# obj$report()

# #show final gradient
# print("final gradient:")
# print(rep$gradient.fixed)
