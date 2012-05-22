pp.fit2 <- function (xdat, threshold, npy = 365, ydat = NULL, mul = NULL, sigl = NULL, shl = NULL, mulink = identity, siglink = identity, shlink = identity, muinit = NULL, siginit = NULL, shinit = NULL, show = TRUE, method = "Nelder-Mead", maxit = 10000, ndeps = 1e-3, exceedancesOnly = TRUE, nBlocks = NULL, propMissingByBlock = 0, ydatByBlock = NULL, thresholdByBlock = NULL, ...) 
{
  # modification of pp.fit() from ismev by C. Paciorek
  # differences from pp.fit()
  # (1) modifies default initial values to be calculated only using exceedances, and modifies initial values for log link
  # (2) allows NA in data, adjusting intensity function calculation in likelihood to account for missing values
  # (3) allows user to provide only the exceedances for computational efficiency; in this case, for nonstationary models, the covariates must not vary within block (e.g., year) as the likelihood needs to calculate the intensity measure appropriate for every timepoint at which an observation is possible. The covariate values, proportion of missing values, and threshold for each block must be supplied when these are relevant
  # (4) returns data values and location, scale and shape values only for the exceedances
  # (5) adds 'flag' output that indicates if MLE or standard errors may not be legitimate
  # (6) allows one to change the default step size of the finite difference approximation to the gradient in the optimization

  # Note that this function operates in two fashions. First (exceedancesOnly = FALSE) it replicates pp.fit() but allows for missing data. In this case the data, threshold values, and covariates supplied should be given for every timepoint. Missing values are not allowed in the covariates or the threshold values as these are need for calculation of the intensity function in the likelihood.  In this case, information about the blocks is ignored. Second (exceedancesOnly = TRUE), it allows one to improve computational efficiency by providing only the exceedances.  In this case, the user is required to provide nBlocks and should provide propMissingByBlock, ydatByBlock, and thresholdByBlock as appropriate.  When only exceedances are supplied there should be no missing values in xdat.

  # for strict accuracy, one should specify npy to be equal to the total number of observation units in the number of blocks to be considered for analysis divided by the number of blocks; e.g., the total number of days in the years of the analysis divided by the number of years; note that blocks in general should be equal size for the parameter estimates and return values to be meaningful across multiple blocks, but slight deviations from, e.g., leap years should have limited effects

  # probably want to have sc and threshold checks for all timepoints not just exceedances, unlike in pp.fit(); for now I'm keeping the checks just for the exceedances, as in pp.fit()
  
  # note possible errors in pp.fit checking of legitimacy of parameters in terms of obs/thresholds not outside domain of distribution
  
  # add upper.tail? no, probably not; but if so give msg that says: provide me the maxima of negative data and that this will handle the results correctly?
  
  z <- list()
  npmu <- length(mul) + 1
  npsc <- length(sigl) + 1
  npsh <- length(shl) + 1

  z$trans <- FALSE 
  if (is.function(threshold)) 
    stop("`threshold' cannot be a function")
  
  covars = FALSE
  if(!is.null(mul) || !is.null(sigl) || !is.null(shl))
    covars = TRUE  # 

  if(covars || length(threshold) > 1)
    z$trans <- TRUE  # nonstationary model
  
  if(exceedancesOnly & is.null(nBlocks))
    stop("number of blocks (e.g., years) (nBlocks) is required for calculation of the likelihood when only exceedances are supplied")

  if(exceedancesOnly){ # check that nBlocks matches dimensionality of byBlock objects
    nBlocksError = FALSE
    if(length(propMissingByBlock) > 1 && length(propMissingByBlock) != nBlocks)
      nBlocksError = TRUE
    if(!is.null(ydatByBlock) && nrow(ydatByBlock) != nBlocks)
      nBlocksError = TRUE
    if(!is.null(thresholdByBlock) && length(thresholdByBlock) != nBlocks)
      nBlocksError = TRUE
    if(nBlocksError)
      stop("number of blocks indicated by nBlocks and one of {propMissingByBlock, ydatByBlock, thresholdByBlock} are in conflict")
  }

  if(exceedancesOnly && covars && is.null(ydatByBlock))
    stop("nonstationary modeling when providing only exceedances requires ydatByBlock to calculate the intensity function portion of the likelihood")
  
  if(exceedancesOnly &&  is.null(thresholdByBlock)){
    if(length(threshold) > 1){
      stop("require thresholdByBlock when threshold varies and only exceedances are supplied")
    } else{
      thresholdByBlock <- rep(threshold, nBlocks)
    }
  }

  if(exceedancesOnly && length(propMissingByBlock) == 1)
    propMissingByBlock = rep(propMissingByBlock, nBlocks)
  
  n <- length(xdat)
  if(exceedancesOnly){
    ny <- nBlocks # code below uses 'ny' since this is used in original pp.fit
    # nyEff <- sum(1-propMissingByBlock)
  } else{
    ny <- n/npy
    nyEff <- sum(!is.na(xdat))/npy
  }

  if(n == 1)
    stop("fitting will not be done correctly with a single observation or single exceedance")

  u <- threshold
  if(length(threshold) == 1)
    u <- rep(threshold, n)
  if(length(u) > 1 & length(u) != n)
    stop("number of threshold values should match number of observations, unless threshold does not vary")

  uInd <- !is.na(xdat) & xdat > u  # indicator of exceedances
  xdatu <- xdat[uInd] 
  u[is.na(xdat)] = NA # sets things up so that intensity is not calculated when data are missing
  nu <- length(xdatu)
  in2 <- sqrt(6 * var(xdatu))/pi  # modified from pp.fit to make use only of exceedances
  in1 <- mean(xdatu) - 0.57722 * in2  # modified from pp.fit to make use only of exceedances

  if(identical(mulink, exp))
    in1 = log(in1)
  if(!identical(mulink, exp) && !identical(mulink, identity))
    warning("Default initial value for location is on incorrect scale; please supply your own.")
  if(identical(siglink, exp))
    in2 = log(in2)
  if(!identical(siglink, exp) && !identical(siglink, identity))
    warning("Default initial value for scale is on incorrect scale; please supply your own.")
  shinit.default = 0.1
  if(identical(shlink, exp))
    shinit.default = log(shinit.default)
  if(!identical(shlink, exp) && !identical(shlink, identity))
    warning("Default initial value for shape is on incorrect scale; please supply your own.")

  if(is.null(mul)) {
    mumat <- as.matrix(rep(1, n))
    if(is.null(muinit)) 
      muinit <- in1
    mumatByBlock <- matrix(rep(1, ny), nc = 1)
  } else {
    mumat <- cbind(rep(1, n), ydat[ , mul])
    mumatByBlock <- cbind(rep(1, ny), ydatByBlock[, mul])
    if(is.null(muinit)) 
      muinit <- c(in1, rep(0, length(mul)))
  }
  if(is.null(sigl)) {
    sigmat <- as.matrix(rep(1, n))
    if(is.null(siginit)) 
      siginit <- in2
    sigmatByBlock <- matrix(rep(1, ny), nc = 1)
  } else {
    sigmat <- cbind(rep(1, n), ydat[ , sigl])
    sigmatByBlock <- cbind(rep(1, ny), ydatByBlock[, sigl])
    if(is.null(siginit)) 
      siginit <- c(in2, rep(0, length(sigl)))
  }
  if(is.null(shl)) {
    shmat <- as.matrix(rep(1, n))
    if(is.null(shinit)) 
      shinit <- shinit.default
    shmatByBlock <- matrix(rep(1, ny), nc = 1)
  } else {
    shmat <- cbind(rep(1, n), ydat[ , shl])
    shmatByBlock <- cbind(rep(1, ny), ydatByBlock[, shl])
    if(is.null(shinit)) 
      shinit <- c(shinit.default, rep(0, length(shl)))
  }
  init <- c(muinit, siginit, shinit)
  z$model <- list(mul, sigl, shl)
  z$link <- deparse(substitute(c(mulink, siglink, shlink)))
  z$threshold <- threshold
  z$npy <- npy
  z$nexc <- nu
  z$data <- xdatu

  pp.lik <- function(a) {
    mu <- c(mulink(mumat %*% (a[1:npmu])))
    sc <- c(siglink(sigmat %*% (a[seq(npmu + 1, length = npsc)])))
    xi <- c(shlink(shmat %*% (a[seq(npmu + npsc + 1, length = npsh)])))
    if(exceedancesOnly){ # calculate parameters for each block for use in intensity function in likelihood
      muByBlock <- c(mulink(mumatByBlock %*% (a[1:npmu])))
      scByBlock <- c(siglink(sigmatByBlock %*% (a[seq(npmu + 1, length = npsc)])))
      xiByBlock <- c(shlink(shmatByBlock %*% (a[seq(npmu + npsc + 1, length = npsh)])))
    }

    # basic logic of the checks below is that the quantities that are exponentiated in (7.9) of Coles (p. 134) should be positive - i.e., for all exceedances and for all threshold values for all timepoints
    
    if(min(sc[uInd]) <= 0 || (exceedancesOnly && min(scByBlock) <= 0)) 
      return(10^6)  # note that pp.fit() only checks min(sc[uInd]) <=0, but presumably the scale should be positive for all potential observations
    
    # check that thresholds do not exceed the bounds of the distribution; pp.fit() does this only for the threshold values corresponding to the exceedances, but it's not clear why this shouldn't be checked for all timepoints
    if (min((1 + xi * (u - mu)/sc)[uInd]) <= 0) 
      return(10^6)
    if(exceedancesOnly && min(1 + (xiByBlock * (thresholdByBlock - muByBlock))/scByBlock) <= 0)
      return(10^6)
      
    y <- (xdatu - mu[uInd])/sc[uInd]
    y <- 1 + xi[uInd] * y
    if(min(y) <= 0){  # check that exceedances do not exceed the bounds of the distribution; note that pp.fit() checks all observations, even those below the threshold, for reasons that are unclear
      l <- 10^6;
    } else{
      l <- sum(log(sc[uInd])) + sum(log(y) * (1/xi[uInd] + 1))  # PP density of the exceedances
      if(exceedancesOnly){  # calculate intensity function for all timepoints; using sum gives parameter estimates that correspond to GEV modeling of single block extrema (this is equivalent to multiplying by ny as in Coles (2001, p. 133) but ignoring the intensity values corresponding to the missing values)
        l <- l + sum( (1 - propMissingByBlock) * (1 + (xiByBlock * (thresholdByBlock - muByBlock))/scByBlock)^(-1/xiByBlock))
      } else{
        l <- l + nyEff * mean((1 + (xi * (u - mu))/sc)^(-1/xi), na.rm = TRUE) # need scaled mean if per-time-point intensity so that parameters correspond GEV modeling of single block extrema
      }
    }
    return(l)
  }
  x <- optim(init, pp.lik, hessian = TRUE, method = method, 
             control = list(maxit = maxit, ndeps = rep(ndeps, length = length(init)), ...))
  # calculate parameters only for the exceedances, unlike pp.fit()
  mu <- mulink(c(mumat[uInd, , drop = FALSE] %*% (x$par[1:npmu])))
  sc <- siglink(c(sigmat[uInd, , drop = FALSE] %*% (x$par[seq(npmu + 1, length = npsc)])))
  xi <- shlink(c(shmat[uInd, , drop = FALSE] %*% (x$par[seq(npmu + npsc + 1, length = npsh)])))
  z$conv <- x$convergence
  z$nllh <- x$value
  z$vals <- cbind(mu, sc, xi, u[uInd])
  z$gpd <- apply(z$vals, 1, ppp, npy) 
  if(z$trans) {
    z$data <- as.vector((1 + (xi * (xdatu - u[uInd]))/z$gpd[2, ])^(-1/xi))
  }
  z$mle <- x$par
  z$cov <- solve(x$hessian)
  z$se <- sqrt(diag(z$cov))
  if(show) {
    if(z$trans) 
      print(z[c(2, 3)])
    if(length(z[[4]]) == 1) 
      print(z[4])
    print(z[c(5, 6, 8)])
    if(!z$conv) 
      print(z[c(9, 12, 14)])
  }
  z$flag = 0
  if(sum(is.na(c(z$mle, z$se))) || min(xi) < -1 || min(z$se) < 1e-5){
    z$flag = 1
    warning("MLEs and standard errors may not be reliable; please check convergence, estimates, standard errors, and input data.")
  }
  class(z) <- "pp.fit"
  invisible(z)
}
