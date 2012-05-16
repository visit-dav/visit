gev.fit2 <- function (xdat, ydat = NULL, mul = NULL, sigl = NULL, shl = NULL, 
    mulink = identity, siglink = identity, shlink = identity, 
    muinit = NULL, siginit = NULL, shinit = NULL, show = TRUE, 
    method = "Nelder-Mead", maxit = 10000, ndeps = 1e-6, ...) 
{
  # modification of gev.fit() from ismev by C. Paciorek
  # differences from gev.fit()
  # (1) has params be scalar when no covariates to improve computational efficiency - this only makes much difference in speed when the data vector is fairly long
  # (2) robustly handles NAs in xdat
  # (3) modifies initial values for log link
  # (4) adds 'flag' output that indicates if MLE or standard errors may not be legitimate
  # (5) allows one to change the default step size of the finite difference approximation to the gradient in the optimization
  
  z <- list()
  npmu <- length(mul) + 1
  npsc <- length(sigl) + 1
  npsh <- length(shl) + 1
  z$trans <- FALSE
  in2 <- sqrt(6 * var(xdat, na.rm=T))/pi
  in1 <- mean(xdat, na.rm=T) - 0.57722 * in2
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

  if (is.null(mul)) {
    mumat <- 1  # as.matrix(rep(1, length(xdat)))
    if (is.null(muinit)) 
      muinit <- in1
  } else {
    z$trans <- TRUE
    mumat <- cbind(rep(1, length(xdat)), ydat[, mul])
    if (is.null(muinit)) 
      muinit <- c(in1, rep(0, length(mul)))
  }
  if (is.null(sigl)) {
    sigmat <- 1 # as.matrix(rep(1, length(xdat)))
    if (is.null(siginit)) 
      siginit <- in2
  } else {
    z$trans <- TRUE
    sigmat <- cbind(rep(1, length(xdat)), ydat[, sigl])
    if (is.null(siginit)) 
      siginit <- c(in2, rep(0, length(sigl)))
  }
  if (is.null(shl)) {
    shmat <- 1  # as.matrix(rep(1, length(xdat)))
    if (is.null(shinit)) 
      shinit <- shinit.default
  } else {
    z$trans <- TRUE
    shmat <- cbind(rep(1, length(xdat)), ydat[, shl])
    if (is.null(shinit)) 
      shinit <- c(shinit.default, rep(0, length(shl)))
  }
  z$model <- list(mul, sigl, shl)
  z$link <- deparse(substitute(c(mulink, siglink, shlink)))
  init <- c(muinit, siginit, shinit)
  gev.lik <- function(a) {
    mu <- c(mulink(mumat %*% (a[1:npmu])))
    sc <- c(siglink(sigmat %*% (a[seq(npmu + 1, length = npsc)])))
    xi <- c(shlink(shmat %*% (a[seq(npmu + npsc + 1, length = npsh)])))
    y <- (xdat - mu)/sc
    y <- 1 + xi * y
    if (any(y <= 0, na.rm = TRUE) || any(sc <= 0)) 
      return(10^6)
    if(length(sc) == 1){
      l <- length(xdat[!is.na(xdat)]) * log(sc)
    } else{
      l <- sum(log(sc[!is.na(xdat)]))
    }
    l <- l + sum(y^(-1/xi), na.rm = TRUE) + sum(log(y) * (1/xi + 1), na.rm = TRUE)
    return(l)
  }
  x <- optim(init, gev.lik, hessian = TRUE, method = method, 
             control = list(maxit = maxit, ndeps = rep(ndeps, length = length(init)), ...))
  z$conv <- x$convergence
  mu <- c(mulink(mumat %*% (x$par[1:npmu])))
  sc <- c(siglink(sigmat %*% (x$par[seq(npmu + 1, length = npsc)])))
  xi <- c(shlink(shmat %*% (x$par[seq(npmu + npsc + 1, length = npsh)])))
  z$nllh <- x$value
  z$data <- xdat
  if (z$trans) {
    z$data <- -log(as.vector((1 + (xi * (xdat - mu))/sc)^(-1/xi)))
  }
  z$mle <- x$par
  z$cov <- solve(x$hessian)
  z$se <- sqrt(diag(z$cov))
  z$vals <- cbind(mu, sc, xi)
  if (show) {
    if (z$trans) 
      print(z[c(2, 3, 4)])
    else print(z[4])
    if (!z$conv) 
      print(z[c(5, 7, 9)])
  }
  z$flag = 0
  if(sum(is.na(c(z$mle, z$se))) || min(xi) < -1 || min(z$se) < 1e-5){
    z$flag = 1
    warning("MLEs and standard errors may not be reliable; please check convergence, estimates, standard errors, and input data.")
  }
  class(z) <- "gev.fit"
  invisible(z)
}
