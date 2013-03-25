gevFit <- function(data, aggregation = "annual", nYears, nReplicates = 1, nCovariates = 0, covariates = NULL, dataScaling = 1, locationModel = NULL, scaleModel = NULL, shapeModel = NULL, missingFlag = NULL, returnParams = FALSE, rvInterval = 20, newData = NULL, rvDifference = NULL, maxes = TRUE, optimMethod = "Nelder-Mead"){
  # data should be a 1-d array containing the maxes (or mins) by year (year x (optionally) month x (optionally) location), with the year index varying fastest and the location index varying slowest. For "seasonal" aggregation, the maxes should be provided by month. For seasonal analysis, data should be given as consecutive years, with NA for missing values, as the code needs to treat December as being with the following year. If there are NAs for any months of a given season, the seasonal value is taken to be NA.
  # aggregation should be one of "annual", "seasonal", or "monthly", indicating the stratification. If monthly or seasonal, separate results will be reported for each stratum (i.e., each month or season)
  # nYears is the number of years (more generally of blocks) of data provided
  # nReplicates is the number of replicate data sets; primarily for use with model output where you can run the model multiple times to get independent replicates
  # nCovariates indicates the number of covariates provided through 'covariates' (note that any subset of the covariates that are provided may be used in the location, scale, and shape modeling, as specified in locationModel, scaleModel, shapeModel)
  # covariates is a 1-d array of covariate values (year x covariate x (optionally) month or season), with the observation index varying fastest and month/season index varying slowest. This should be NULL if 'nCovariates' is 0.
  # dataScaling is a positive-valued scalar used to scale the data values for more robust optimization performance. When multiplied by the values, it should produce values with magnitude around 1
  # locationModel, scaleModel, and shapeModel are vectors indicating the indices of the covariates to be used in the location, scale and shape parameterization. The values are used to select columns from the 'covariates' array after they are transformed to multidimensional arrays with the second dimension indexing the covariates
  # missingFlag is a value to be interpreted as missing values (NA in R), intended for use in other languages calling this function
  # returnParams is a boolean indicating whether to return the fitted parameter values and their standard errors; WARNINGS: (1) Parameter values for models with covariates must be interpreted based on transforming each covariate by subtracting the mean of the yearly values (from 'covariatesByYear') and dividing by the difference of the max and min of the yearly values. This scales the covariates for better numerical performance in the optimization. (2) parameter values for models with covariates for the scale parameter must interpreted based on the log transformation of the scale parameter
  # rvInterval: the timespan for which return values should be calculated. For example a rvInterval of 20 years corresponds to the value of an event that occurs with probability 1/20 in any year and therefore occurs on average every 20 years
  # newData should be a 1-d array providing covariate values (observation x covariate) for which return values are desired. Values will be calculated for each stratum.
  # rvDifference should be a 1-d array of covariate values for two sets of covariates for which the difference in return values is desired (set x covariate), with the set index varying fastest; i.e. provide the first covariate for each set, then the second covariate for each set, etc. The difference is computed as the return value for the second set minus the return value for the first set. Values will be calculated for each stratum.
  # maxes should be TRUE when analyzing block maxima and FALSE when analyzing block minima
  # optimMethod allows you to use different methods as specified in 'optim'; by default this is the derivative-free Nelder-Mead method

  require(ismev) # won't be needed when start using gev.fit2
    
  if(!maxes){  # modeling minima is equivalent to modeling the negative of maxima. Location parameter values will be the negative of those on the original scale, but are corrected before returning parameter values to the user.
    data = -data
  }

  if(!is.null(missingFlag)){
    data[data == missingFlag] <- NA
    if(!is.null(covariates))
     covariates[covariates == missingFlag] <- NA
  }

  nStrata = 1
  if(aggregation == "monthly"){
    nStrata = 12
  }
  if(aggregation == "seasonal"){
    nStrata = 4
  }
  
  data <- data * dataScaling

  # check dimensionality of input arrays
  if((aggregation == "annual" && length(data) != nYears*nReplicates) ||
     (aggregation != "annual" && length(data) != nYears*12*nReplicates))
    stop("length of input data does not match number of years, covariates, and months (the latter is required for seasonal and monthly analyses")
  if(!is.null(covariates) && !(length(covariates) %in% (nYears*nCovariates*c(1, nStrata))))
    stop("supplied 'covariate' values do not match number of years, covariates and strata")
  if(!is.null(newData) && length(newData) %% nCovariates != 0)
    stop("length of newData should be a multiple of 'nCovariates'")
  if(!is.null(rvDifference) && length(rvDifference) != 2*nCovariates)
    stop("length of rvDifference is not equal to two times the number of covariates")
  
  # manipulate input arrays to have appropriate number of dimensions
  if(aggregation == "annual"){
    data <- array(data, c(nYears, nStrata, nReplicates))
  } else{
    data <- array(data, c(nYears, 12, nReplicates))
  }
  if(!is.null(covariates))
    covariates <- array(covariates, c(nYears, nCovariates, nStrata))
  if(!is.null(newData)){
    m = length(newData)/nCovariates # number of return values to compute
    newData <- array(newData, c(m, nCovariates))
  }
  if(!is.null(rvDifference))
    rvDifference <- array(rvDifference, c(2, nCovariates))
  
  if(is.null(rvInterval) && (!is.null(newData) || !is.null(rvDifference)))
    stop("'rvInterval' must be specified")
  
  if(aggregation == 'seasonal'){
    seasons <- c('DJF', 'MAM', 'JJA', 'SON')
    data <- seasonalize(data, maxes = TRUE)
  }
  
  nParam <- 3 + length(locationModel) + length(scaleModel) + length(shapeModel)
  
  if(!is.null(locationModel) && !validateIndices(locationModel, nCovariates))
    stop("'locationModel' values do not provide legitimate indices of covariates")
  
  if(!is.null(scaleModel) && !validateIndices(scaleModel, nCovariates))
    stop("'scaleModel' values do not provide legitimate indices of covariates")
  
  if(!is.null(shapeModel) && !validateIndices(shapeModel, nCovariates))
    stop("'shapeModel' values do not provide legitimate indices of covariates")

  if(nCovariates){
    for(p in 1:nCovariates){ # shift and scale to (-.5, .5) for better numeric properties in estimation
      if(!is.null(newData))
        newData[ , p] <- normalize(newData[, p], mean(covariates[ , p, ]), min(covariates[ , p, ]), max(covariates[ , p, ]))
      if(!is.null(rvDifference))
        rvDifference[ , p] <- normalize(rvDifference[, p], mean(covariates[ , p, ]), min(covariates[ , p, ]), max(covariates[ , p, ]))
      covariates[ , p, ] <- normalize(covariates[ , p, ])
    }
  }
  # do I need to save the original covariates or at least mean and divisor of normalization?

  mulink <- siglink <- shlink <- identity
  link = "c(identity, identity, identity)"
  if(!is.null(scaleModel)){
    siglink <- exp
    link = "c(identity, exp, identity)"
  }
  
  NAlist <- list(mle = rep(NA, nParam), se = rep(NA, nParam), cov = matrix(NA, nParam, nParam)) 

  gev.fit.wrap <- function(xdat, ydat){
    fit <- try(gev.fit2(xdat, ydat = ydat, mul = locationModel, sigl = scaleModel, shl = shapeModel, mulink = mulink, siglink = siglink, shlink = shlink, show = FALSE))
    if(is(fit, 'try-error') || fit$conv || fit$flag)
      fit <- NAlist
    return(fit)
  }

  extract <- function(index, object, name)
    object[[index]][[name]]

  mle <- se <- array(NA, c(nParam, nStrata))
  covmat <- array(NA, c(nParam, nParam, nStrata))

  for(j in 1:nStrata){
    output <- gev.fit.wrap(c(data[ , j, ]), matrix(rep(c(t(covariates[ , , j])), nReplicates), ncol = nCovariates, byrow = TRUE))
    mle[ , j] <- output[["mle"]]
    if(!maxes)  # location parameters for minima are the negative of those computed based on negative of minima
      mle[1:(length(locationModel)+1), ] <- -mle[1:(length(locationModel)+1), ]
    se[ , j] <- output[["se"]]
    covmat[ , , j] <- output[["cov"]]
  }

  results <- list()
  numLocScaleParams = 2 + length(locationModel) + length(scaleModel)
  # rescale parameters so on scale of original data
  mle[1:numLocScaleParams, ] <- mle[1:numLocScaleParams, ] / dataScaling
  se[1:numLocScaleParams, ] <- se[1:numLocScaleParams, ] / dataScaling
  
  if(returnParams){
    if(aggregation == 'seasonal')
      attributes(mle)$dimnames[[2]] <- attributes(se)$dimnames[[2]] <- seasons
    results$mle <- mle[ , , drop = TRUE]
    results$se.mle <- se[ , , drop = TRUE]
  }
  
  # get return values for newData observations
  # perhaps make this more efficient with an apply, but it needs to pass in both mle and cov
  if(!is.null(newData)){
    rv <- array(0, c(m, nStrata, 2))
    for(i in 1:m)
      for(j in 1:nStrata){
        fit = list(mle = mle[ , j], cov = covmat[ , , j], model = list(locationModel, scaleModel, shapeModel), link = link) 
        class(fit) = "gev.fit"
        rv[i, j, ] <- returnValue(fit, rvInterval, newData[i, ])
      }
    if(aggregation == "seasonal")
      attributes(rv)$dimnames[[2]] <- seasons
    results$returnValue <- rv[ , , 1]
    results$se.returnValue <- rv[ , , 2]
  }

  # get stationary return value
  if(is.null(newData) && !is.null(rvInterval) && is.null(covariates)){ 
    rv <- array(0, c(nStrata, 2))
    for(j in 1:nStrata) {
      fit = list(mle = mle[ , j], cov = covmat[ , , j], model = list(locationModel, scaleModel, shapeModel), link = link)
      class(fit) = "gev.fit"
      rv[j, ] <- returnValue(fit, rvInterval, rvCovariates = NULL)
    }
    if(aggregation == "seasonal")
      attributes(rv)$dimnames[[1]] <- seasons
    results$returnValue <- rv[ , 1]
    results$se.returnValue <- rv[ , 2]
  }
  
  # get return value difference
  if(!is.null(rvDifference)){
    rvDiff <- array(0, c(nStrata, 2))
    for(j in 1:nStrata) {
      fit = list(mle = mle[ , j], cov = covmat[ , , j], model = list(locationModel, scaleModel, shapeModel), link = link)
      class(fit) = "gev.fit"
      rvDiff[j, ] <- returnValueDiff(fit, rvInterval, rvDifference)
    }
    if(aggregation == "seasonal")
      attributes(rvDiff)$dimnames[[1]] <- seasons
    results$returnValueDiff <- rvDiff[ , 1]
    results$se.returnValueDiff <- rvDiff[ , 2]
  }
 
  return(results)
}
