seasonalize <- function(arr, maxes = TRUE){
  # this function finds the max or min by climatological season (DJF, MAM, JJA, SON), assuming a 3-d array with month as the second dimension

  if(maxes) FUN <- max else FUN <- min
  
  nSeas <- 4
  seasonIndices <- list(DJF = c(1, 2, 12), MAM = 3:5, JJA = 6:8, SON = 9:11)
  dec <- 12
  nYr <- dim(arr)[1]
  
  arr[2:nYr, dec, ] <- arr[1:(nYr - 1), dec, ]
  arr[1, dec, ] <- NA

  tmp <- array(NA, c(nYr, nSeas, dim(arr)[3]))
  for(j in 1:nSeas){
    tmp[ , j, ] <- apply(arr[, seasonIndices[[j]], , drop = FALSE], c(1, 3), FUN, na.rm = FALSE)
  }
  return(tmp)
}

is.wholenumber <- function(x, tol = .Machine$double.eps^0.5)  abs(x - round(x)) < tol

validateIndices <- function(model, nVar){
  if(min(model) < 0 | max(model) > nVar)
    return(FALSE)
  if(sum(is.wholenumber(model)) < length(model))
    return(FALSE)
  if(length(unique(model)) != length(model))
    return(FALSE)
  return(TRUE)
}

normalize <- function(vec, shift = NULL, lower = NULL, upper = NULL){
  if(is.null(shift))
    shift <- mean(vec)
  if(is.null(lower))
    lower <- min(vec)
  if(is.null(upper))
    upper <- max(vec)
  (vec - shift) / (upper - lower)
}

returnValue <- function(fit, rvInterval = 20, rvCovariates = NULL){

  if(!(class(fit) == 'gev.fit' | class(fit) == 'pp.fit'))
    stop('Function only handles gev.fit and pp.fit objects')

#  if(!identical(fit$link[[1]], identity) || !identical(shlink, identity) ||  == 'gev.fit' | class(fit) == 'pp.fit'))
  #  stop('Function only handles gev.fit and pp.fit objects')
  
  links = substring(fit$link, 3, nchar(fit$link) - 1)
  links = unlist(strsplit(links, ", "))

  if(links[1] != "identity" || links[3] != "identity" || !(links[2] %in% c("identity", "exp")))
    stop("Function only computes return values for identity link functions for location and shape and for identity or exponential link functions for scale")

  locationIndices <- 1:(length(fit$model[[1]]) + 1)
  pos = 1 + max(locationIndices)
  scaleIndices <- pos:(pos+length(fit$model[[2]]))
  pos = 1 + max(scaleIndices)
  shapeIndices <- pos:(pos+length(fit$model[[3]]))
    
  location <- eval(as.name(links[1]))(sum(c(1, rvCovariates[fit$model[[1]]]) * fit$mle[locationIndices]))
  scale <- eval(as.name(links[2]))(sum(c(1, rvCovariates[fit$model[[2]]]) * fit$mle[scaleIndices]))
  shape <- eval(as.name(links[3]))(sum(c(1, rvCovariates[fit$model[[3]]]) * fit$mle[shapeIndices]))

  yp <- -log(1 - 1/rvInterval)
  ypshape <- yp^(-shape)
  zp <- location - (scale/shape) * (1 - ypshape)

  if(links[2] == "identity"){
    scaleTerm = 1
  } else{  # exponential link
    scaleTerm = scale
  }
       
  grad <- c(c(1, rvCovariates[fit$model[[1]]]),
    -(scaleTerm/shape) * (1 - ypshape) * c(1, rvCovariates[fit$model[[2]]]),
    ((scale/shape^2) * (1 - ypshape) - (scale/shape) * ypshape * log(yp)) * c(1, rvCovariates[fit$model[[3]]]))

  se <- sqrt(t(grad) %*% fit$cov %*% grad)
  
  return(c(zp, se))
}

# difference in return values for single time series, different covariate values
returnValueDiff <- function(fit, rvInterval = 20, rvCovariates = NULL){
  if(!(class(fit) == 'gev.fit' || class(fit) == 'pp.fit'))
    stop('Function only handles gev.fit and pp.fit objects')
  
  links = substring(fit$link, 3, nchar(fit$link) - 1)
  links = unlist(strsplit(links, ", "))
  
  if(links[1] != "identity" || links[3] != "identity" || !(links[2] %in% c("identity", "exp")))
    stop("Function only computes return value differences for identity link functions for location and shape and for identity or exponential link functions for scale")

  locationIndices <- 1:(length(fit$model[[1]]) + 1)
  pos = 1 + max(locationIndices)
  scaleIndices <- pos:(pos+length(fit$model[[2]]))
  pos = 1 + max(scaleIndices)
  shapeIndices <- pos:(pos+length(fit$model[[3]]))
  
  location <- eval(as.name(links[1]))(cbind(rep(1, 2), rvCovariates[ , fit$model[[1]]]) %*% fit$mle[locationIndices])
  scale <- eval(as.name(links[2]))(cbind(rep(1, 2), rvCovariates[ , fit$model[[2]]]) %*% fit$mle[scaleIndices])
  shape <- eval(as.name(links[3]))(cbind(rep(1, 2), rvCovariates[ , fit$model[[3]]]) %*% fit$mle[shapeIndices])
  
  yp <- -log(1 - 1/rvInterval)
  ypshape <- yp^(-shape)
  zpDiff <- diff(location - (scale/shape) * (1 - ypshape))

  if(links[2] == "identity"){
    scaleTerm = rep(1, 2)
  } else{  # exponential link
    scaleTerm = rep(scale, 2)
  }

  grad <- c(0, rvCovariates[2, fit$model[[1]]] - rvCovariates[1, fit$model[[1]]],
    -(scaleTerm[2]/shape[2]) * (1 - ypshape[2]) * c(1, rvCovariates[2, fit$model[[2]]]) +
    (scaleTerm[1]/shape[1]) * (1 - ypshape[1]) * c(1, rvCovariates[1, fit$model[[2]]]),
    ((scale[2]/shape[2]^2) * (1 - ypshape[2]) - (scale[2]/shape[2]) * ypshape[2] * log(yp) ) * c(1, rvCovariates[2, fit$model[[3]]]) -
    ((scale[1]/shape[1]^2) * (1 - ypshape[1]) - (scale[1]/shape[1]) * ypshape[1] * log(yp) ) * c(1, rvCovariates[1, fit$model[[3]]]))
    
  se <- sqrt(t(grad) %*% fit$cov %*% grad)
  
  return(c(zpDiff, se))
}

removeRuns <- function(vals, days, maxes = TRUE){
  n <- length(vals)
  if(maxes) FUN <- max else FUN <- min
  if(n > 1){
    checkBack <- c(TRUE, days[2:n] - days[1:(n-1)] != 1)
    checkFwd <- c(days[1:(n-1)] - days[2:n] == -1, FALSE)
    seqStarts <- which(checkBack & checkFwd)
    for(start in seqStarts){
      pos = start
      while(pos < n && days[pos + 1] - days[start] == pos + 1 - start) pos <- pos + 1
      tmp <- vals[start:pos]
      jitter = rnorm(length(tmp), 0, 1e-12)  # amounts to randomly choosing a max when values are equal
      tmp[tmp + jitter < FUN(tmp + jitter)] <- NA
      vals[start:pos] <- tmp
    }
  }
  return(vals)
}

# alternative: function for removing multiple exceedances in blockLen-day windows; surprisingly this is much slower
withinBlockScreenAlt <- function(vals, days, blockLen = 10, maxes = TRUE){
  n <- length(vals)
  if(n > 1){
    jitter = rnorm(n, 0, 1e-12) # randomly choose which value in block to retain when there are ties
    if(maxes) FUN <- max else FUN <- min
    block <- data.frame(block = cut(days, seq(0, max(days)+blockLen, by = blockLen)))
    tmp <- tapply(vals + jitter, block, FUN)
    blockExtrema <- data.frame(block = names(tmp), extrema = tmp)
    comparators = merge(block, blockExtrema, all.x = TRUE, all.y = FALSE)
    vals[vals + jitter < comparators$extrema] = NA
  }
  return(vals)
}

withinBlockScreen <-  function(vals, days, blockLen = 10, maxes = TRUE){
  if(!identical(days, sort(days)))
    stop('data must be sorted by days in ascending order')
  n <- length(vals)
  if(n > 1){
    i <- 1
    while(i < length(vals)){
      start <- i
      while( (days[i+1]-1) %/% blockLen == (days[i]-1) %/% blockLen &&  i < length(vals)){
        i <- i + 1
      }
      if(i > start){
        jitter = rnorm(i - start + 1, 0, 1e-12)
        tmp = vals[start:i]
        tmp[tmp + jitter < max(tmp + jitter)] = NA
        vals[start:i] = tmp
      }
      i = i + 1
    }
  }
  return(vals)
}

lik.ratio.test = function(fit1, fit2){
  if(!(class(fit1) %in% c("gev.fit", "pp.fit")) ||
     !(class(fit2) %in% c("gev.fit", "pp.fit")))
    stop("Fit objects must be of class 'gev.fit' or 'pp.fit'")
  dev = 2 * abs(fit1$nllh - fit2$nllh)
  df = abs(length(unlist(fit1$model)) - length(unlist(fit2$model)))
  return(list(df = df, negLogLik = c(fit1$nllh, fit2$nllh), chisq = dev, p = 1 - pchisq(dev, df)))
}





#linkManip = function(mulink, siglink, shlink)
# return(deparse(substitute(c(mulink, siglink, shlink))))
