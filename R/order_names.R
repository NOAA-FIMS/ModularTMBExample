order_names <- function(name_map){

    name_order <- unname(name_map)
    ordered_names <- c()
    for(i in 0:(length(name_map)-1)){
        ordered_names[i+1] <- names(name_map)[which(name_order==i)]
    }
    return(ordered_names)
}