simulate_data <- function(seed = 2342, nfish = 10, ages = 1:15, 
l_inf = 10.0, k = 0.5, l_inf_sd = 0.1, k_sd = 0.1, a_min = 0.1){
    set.seed(seed)

nreps <- length(ages)
## parameters per fish
log_l_inf <- rnorm(nfish, log(l_inf), sd=l_inf_sd)
log_k <- rnorm(nfish, log(k), sd=k_sd)
a_min <- a_min
fish <- rep(1:nfish, each=nreps)
## simulate
obs <- lapply(1:nfish, function(i)
  data.frame(fish=i, age=ages,
        length=(exp(log_l_inf[i]))*(1.0-exp(-exp(log_k[i])*(ages-a_min))))) %>%
  bind_rows %>%
  mutate(obs=length*exp(rnorm(nfish*nreps,0-.05^2/2,.05)))

  return(obs)
}