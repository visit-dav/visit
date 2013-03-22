potFit <- function(data, day = NULL, month = NULL, year = NULL, initialYear = NULL, aggregation = "annual", nYears, nReplicates = 1, numPerYear = switch(aggregation, annual = 365, c(31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31)), threshold = NULL, nCovariates = 0, covariatesByYear = NULL, propMissing = 0, thresholdByYear = NULL, dataScaling = 1, locationModel = NULL, scaleModel = NULL, shapeModel = NULL, returnParams = FALSE, rvInterval = 20, newData = NULL, rvDifference = NULL, multiDayEventHandling = NULL, upper.tail = TRUE, optimMethod = "Nelder-Mead"){

  # data, day, month, and year are 1-d arrays giving the observed values and corresponding day index and month and year for the exceedances; note that the day index is NOT the day of the year but a continuous index with arbitrary starting value for the first day of the first year. 'day' is required when 'multiDayEventHandling' is not NULL; 'month' is required for seasonal or monthly analyses. 'year' is required when using covariates as 'covariatesByYear' are matched to 'data' based on 'year' and 'initialYear'. With replicated data for which multiDayEventHandling is not NULL, these should be ordered in blocks such that all the exceedances for the first replicate are first, then for the second replicate, etc. That will prevent events from different replicates as being treated as the same event in all but unusual circumstances.
  # initialYear indicates the first year of the dataset; in some cases this may not be the minimum value in 'year', as no exceedances may have occurred in the first year; required when covariates are used
  # aggregation should be one of "annual", "seasonal", or "monthly", indicating the stratification. If monthly or seasonal, separate results will be reported for each stratum (i.e., each month or season)
  # nYears is the number of years (blocks) in the full dataset (the dataset before the exceedances are extracted)
    # nReplicates is the number of replicate data sets; primarily for use with model output where you can run the model multiple times to get independent replicates
  # numPerYear is the number of days in each year. For annual analyses this should be about 365 (i.e., the average of the number of days in the years, including leap years). For seasonal and monthly analyses, this should be a 1-d array with 12 values, one for each month; the default values assume leap years are one-quarter of the years in the full dataset, thus 28.25 days per February. Strictly speaking this should not vary by year as the interpretation of return values is affected, though the effect of leap years should be minimal. 
  # threshold is a scalar for annual analyses, a 1-d array of 4 values for seasonal (winter, spring, summer, fall thresholds) or a 1-d array of 12 values for monthly analyses (Jan thru Dec thresholds). However, if the thresholds vary by year, threshold should be NULL and thresholdByYear should be specified
  # nCovariates indicates the number of covariates provided through 'covariatesByYear' (note that any subset of the covariates that are provided may be used in the location, scale, and shape modeling, as specified in locationModel, scaleModel, shapeModel)
  # covariatesByYear must be provided if 'nCovariates' is non-zero and must specify the covariate values by year (year x covariate x (optionally) stratum), with the year index varying fastest and stratum index varying slowest
  # thresholdByYear is an (optional) 1-d array of threshold values by year (year x (optionally) stratum), with the year index varying fastest. If the threshold changes over time, this must be included, in which case threshold should be NULL
 # propMissing is either 0 or a 1-d array indicating the proportion of missing values (year x month (for monthly/seasonal analyses)), with the year index varying fastest.  This should include values for all years between the first and last years - years that are entirely missing should have a value of 1
  # dataScaling is a positive-valued scalar used to scale the data values for more robust optimization performance. When multiplied by the values, it should produce values with magnitude around 1
  # locationModel, scaleModel, and shapeModel are vectors indicating the indices of the covariates to be used in the location, scale and shape parameterization. The values are used to select columns from the 'covariatesByYear' array after they are transformed to a multidimensional array with the second dimension indexing the covariates
  # returnParams is a boolean indicating whether to return the fitted parameter values and their standard errors; WARNINGS: (1) Parameter values for models with covariates must be interpreted based on transforming each covariate by subtracting the mean of the yearly values (from 'covariatesByYear') and dividing by the difference of the max and min of the yearly values. This scales the covariates for better numerical performance in the optimization. (2) parameter values for models with covariates for the scale parameter must interpreted based on the log transformation of the scale parameter
  # rvInterval: the timespan for which return values should be calculated. For example a rvInterval of 20 years corresponds to the value of an event that occurs with probability 1/20 in any year and therefore occurs on average every 20 years
  # newData should be a 1-d array of the same form as 'covariatesByYear', providing covariate values (observation x covariate) for which return values are desired. Values will be calculated for each stratum.
  # rvDifference should be a 1-d array of covariate values for two sets of covariates for which the difference in return values is desired (set x covariate), with the set index varying fastest; i.e. provide the first covariate for each set, then the second covariate for each set, etc. The difference is computed as the return value for the second set minus the return value for the first set. Values will be calculated for each stratum.
  # multiDayEventHandling should be NULL, "noruns", or a number. If 'noruns' is specified, only the maximum (or minimum if upper.tail = FALSE) value within a set of exceedances occuring on consecutive days is included. If a number, this should indicate the block size within which to allow only the largest (or smallest if upper.tail = FALSE) value
  # upper.tail indicates whether one is working with exceedances over a high threshold (TRUE) or exceedances under a low threshold (FALSE); in the latter case, the function works with the negative of the values and the threshold, changing the sign of the resulting location parameters
  # optimMethod is passed to the R's optimization routine optim() and should specify one of options for the 'method' argument of optim().  It is advisable to try multiple methods: Nelder-Mead and BFGS are commonly used methods; Nelder-Mead uses only function evaluations, while BFGS is a quasi-Newton method that also uses derivative information.

  # may want to have thresholdByMonth for cases where threshold varies only by stratum and not by observation 
  
  if(!upper.tail){  # modeling exceedances below a threshold is equivalent to modeling the negative of exceedances above the negative of the threshold. Location parameter values will be the negative of those on the original scale, but are corrected before returning parameter values to the user.
    data = -data
    if(!is.null(threshold))
      threshold = -threshold
    if(!is.null(thresholdByYear))
      thresholdByYear = -thresholdByYear
  }
  
  n <- length(data)  
  if(!is.null(day) && length(day) != n)
    stop("'day' must be of same length as 'data', with one value per observation")
  if(!is.null(month) && length(month) != n)
    stop("'month' must be of same length as 'data', with one value per observation")
  if(!is.null(year) && length(year) != n)
    stop("'year' must be of same length as 'data', with one value per observation")
  if(!is.null(threshold))
    if((aggregation == "annual" && length(threshold) != 1) ||
       (aggregation == "seasonal" && length(threshold) != 4) ||
       (aggregation == "monthly" && length(threshold) != 12))
      stop("'threshold' must contain one value for annual, four values for seasonal, or 12 values for montly analysis")

    
  if(!is.null(multiDayEventHandling) && is.null(day))
    stop("'day' is required when multi-day events are excluded")
  if(aggregation != "annual" && is.null(month))
    stop("'month' is required when monthly or seasonal analyses are requested")

  # determine strata-specific information
  nStrata = 1
  if(aggregation == "monthly"){
    nStrata = 12
    if(length(propMissing) > 1 && !(length(propMissing) %in% (nYears*c(1,12))))
      stop("'propMissing' should be a scalar, a vector of one value per year, or a vector with values for each month and year")
    propMissing = array(propMissing, c(nYears, 12))
    if(length(numPerYear) != 12)
      stop("'numPerYear' should have one value for each month")
  }
  if(aggregation == "seasonal"){
    nStrata = 4
    if(length(propMissing) > 1 && !(length(propMissing) %in% (nYears*c(1,12))))
      stop("'propMissing' should be a scalar, a vector of one value per year, or a vector with values for each month and year")
    propMissing = array(propMissing, c(nYears, 12))
    if(length(numPerYear) != 12)
      stop("'numPerYear' should have one value for each month")
  }
  if(aggregation == "annual"){
    propMissing = array(propMissing, c(nYears, 1))
    if(length(numPerYear) != 1)
      stop("'numPerYear' should be a single value for annual analyses")
  }
  
  data <- data * dataScaling
  if(!is.null(threshold))
    threshold = threshold * dataScaling
  if(!is.null(thresholdByYear))
    thresholdByYear = thresholdByYear * dataScaling

  # check dimensionality of input arrays
  if(!is.null(covariatesByYear) && !(length(covariatesByYear) %in% (nYears*nCovariates*c(1,nStrata))))
    stop("'covariatesByYear must have covariate values for each covariate for each year (and optionally stratum)")
  if(!is.null(thresholdByYear) && !(length(thresholdByYear) %in% (nYears*c(1,nStrata))))
    stop("'thresholdByYear must have values for each year (and optionally stratum)")
  if(!is.null(newData) && length(newData) %% nCovariates != 0)
    stop("length of newData should be a multiple of 'nCovariates'")
  if(!is.null(rvDifference) && length(rvDifference) != 2*nCovariates)
    stop("length of rvDifference is not equal to two times the number of covariates")

  # manipulate input arrays to have appropriate number of dimensions and get covariate vals for obs
  if(!is.null(covariatesByYear)) {
    covariatesByYear <- array(covariatesByYear, c(nYears, nCovariates, nStrata))
    if(aggregation == "annual")
      covariates <- matrix(covariatesByYear[(year - initialYear + 1), , 1], ncol = nCovariates)
    if(aggregation == "seasonal"){ # move December to next year
      monthToSeason <- c(1,1,2,2,2,3,3,3,4,4,4,1)
      lastDec = year == (initialYear + nYears - 1) & month == 12
      data = data[!lastDec]
      n <- length(data)
      month = month[!lastDec]
      year = year[!lastDec]
      day = day[!lastDec]
      year[month == 12] = year[month == 12] + 1 
      covariates <- matrix(covariatesByYear[cbind(
                                       rep((year - initialYear + 1), nCovariates),
                                       rep(1:nCovariates, each = n),
                                       rep(monthToSeason[month], nCovariates))],
                           ncol = nCovariates)
    }
    if(aggregation == "monthly")
      covariates <- matrix(covariatesByYear[cbind(
                                       rep((year - initialYear + 1), nCovariates),
                                       rep(1:nCovariates, each = n),
                                       rep(month, nCovariates))],
                           ncol = nCovariates)
  } else{
    covariates <- NULL
  }
  if(!is.null(thresholdByYear))
    thresholdByYear <- array(thresholdByYear, c(nYears, nStrata))
  if(!is.null(newData)){
    m = length(newData)/nCovariates
    newData <- array(newData, c(m, nCovariates))
  }
  if(!is.null(rvDifference))
    rvDifference <- array(rvDifference, c(2, nCovariates))

  if(nCovariates && is.null(covariatesByYear))
    stop("'covariatesByYear' is required for nonstationary modeling")

  if(is.null(rvInterval) && (!is.null(newData) || !is.null(rvDifference)))
    stop("'rvInterval' must be specified")
    
  # seasonalize
  if(aggregation == 'seasonal' && !is.null(covariatesByYear)){
    seasons <- c('DJF', 'MAM', 'JJA', 'SON')
    dec = 12
    months = list(DJF = c(1,2,12), MAM = 3:5, JJA = 6:8, SON = 9:11)
    # put propMissing for first year based on missing Dec and shift other years
    propMissing[2:nYears, dec] = propMissing[1:(nYears-1), dec]
    propMissing[1, dec] = 1
    season = rep(1, n)
    for(seas in 2:4)
      season[month %in% months[[seas]]] = seas
   
    tmp = array(0, c(nYears, 4))
    for(seas in 1:4){
      wgts = numPerYear
      wgts[-months[[seas]]] = 0
      tmp[ , seas] = (propMissing%*%wgts) / sum(wgts)
    }
    propMissing = tmp
    numPerYear = c(sum(numPerYear[months[[1]]]), sum(numPerYear[months[[2]]]), sum(numPerYear[months[[3]]]), sum(numPerYear[months[[4]]]))
  }

  
  # remove multi-day events
  if(is.character(multiDayEventHandling) && multiDayEventHandling == "noruns")
    data = removeRuns(data, day)
  if(is.numeric(multiDayEventHandling))
    data = withinBlockScreen(data, day, blockLen = multiDayEventHandling)

  if(!is.null(month))
    month = month[!is.na(data)]
  if(!is.null(year))
    year = year[!is.na(data)]
  
  if(aggregation == "seasonal")
    season = season[!is.na(data)]

  if(nCovariates)
    covariates <- covariates[!is.na(data), , drop = FALSE]
  data = data[!is.na(data)]
  
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
        newData[ , p] <- normalize(newData[, p], mean(covariatesByYear[ , p, ]), min(covariatesByYear[ , p, ]), max(covariatesByYear[ , p, ]))
      if(!is.null(rvDifference))
        rvDifference[ , p] <- normalize(rvDifference[, p], mean(covariatesByYear[ , p, ]), min(covariatesByYear[ , p, ]), max(covariatesByYear[ , p, ])) 
      covariates[ , p] <- normalize(covariates[ , p], mean(covariatesByYear[ , p, ]), min(covariatesByYear[ , p, ]), max(covariatesByYear[ , p, ]))
      covariatesByYear[ , p, ] <- normalize(covariatesByYear[ , p, ])
    }
  }
  # do I need to save the original covariates or at least mean and divisor of normalization?

  mulink <- siglink <- shlink <- identity
  link = "c(identity, identity, identity)"
  if(!is.null(scaleModel)){
    siglink <- exp
    link = "c(identity, exp, identity)"
  }
  
  NAlist <- list(mle = rep(NA, nParam), se = rep(NA, nParam), cov = matrix(NA, nParam, nParam)) #  rep(NA, nParam)

  pot.fit.wrap = function(xdat, threshold, npy, ydat, ydatByYear, propMissing, nYears, thresholdByYear){
    fit = try(pp.fit2(xdat, threshold = threshold, npy = npy,  ydat = ydat, mul = locationModel, sigl = scaleModel, shl = shapeModel, mulink = mulink, siglink = siglink, shlink = shlink, show = FALSE, exceedancesOnly = TRUE, nBlocks = nYears, propMissingByBlock = propMissing, ydatByBlock = ydatByYear, thresholdByBlock = thresholdByYear))
    if(!is(fit, 'try-error') && !fit$flag && !fit$conv){
      return(fit[c("mle", "se", "cov")])
    } else{
      return(NAlist)
    }
  }

  mle <- se <- array(NA, c(nParam, nStrata))
  covmat <- array(NA, c(nParam, nParam, nStrata))

  for(j in 1:nStrata){
    tmpdata = data
    tmpcovariates = covariates
    tmpcovariatesByYear = covariatesByYear
    if(nCovariates)
      tmpcovariatesByYear = matrix(covariatesByYear[ , , j], nc = nCovariates)      
    if(aggregation == "monthly"){
      tmpdata = data[month == j]
      if(nCovariates){
        tmpcovariates = covariates[month == j, , drop = FALSE]
      } 
    }  
    if(aggregation == "seasonal"){
      tmpdata = data[season == j]
      if(nCovariates){
        tmpcovariates = covariates[season == j, , drop = FALSE]
      } 
    }
    if(nCovariates && nReplicates > 1)
      tmpcovariatesByYear = matrix(rep(c(t(tmpcovariatesByYear)), nReplicates), ncol = nCovariates, byrow = TRUE)

    output = pot.fit.wrap(tmpdata, threshold[j], numPerYear[j], tmpcovariates, tmpcovariatesByYear, rep(propMissing[ , j], nReplicates), nYears*nReplicates, rep(thresholdByYear[ , j], nReplicates))  
    mle[ , j] <- output$mle
    if(!upper.tail)  # location parameters for lower tail are the negative of those computed based on negative of data values
      mle[1:(length(locationModel)+1), ] <- -mle[1:(length(locationModel)+1), ]
    se[ , j] <- output$se
    covmat[ , , j] <- output$cov
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
        class(fit) = "pp.fit"
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
    for(j in 1:nStrata){
      fit = list(mle = mle[ , j], cov = covmat[ , , j], model = list(locationModel, scaleModel, shapeModel), link = link)
      class(fit) = "pp.fit"
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
    for(j in 1:nStrata){
        fit = list(mle = mle[ , j], cov = covmat[ , , j], model = list(locationModel, scaleModel, shapeModel), link = link)
        class(fit) = "pp.fit"
        rvDiff[j, ] <- returnValueDiff(fit, rvInterval, rvDifference) 
      }
    if(aggregation == "seasonal")
      attributes(rvDiff)$dimnames[[1]] <- seasons
    results$returnValueDiff <- rvDiff[ , 1] 
    results$se.returnValueDiff <- rvDiff[ , 2]
  }
  
  return(results)
}

