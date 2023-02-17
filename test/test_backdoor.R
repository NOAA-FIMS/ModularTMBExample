## to do


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
