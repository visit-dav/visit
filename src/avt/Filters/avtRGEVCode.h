#include <string>

static std::string rcode =
"require(ismev)\n"\
"gevFit = function(maxes, aggregation = 'annual', years = NULL, locationModel = 0, scaleModel = 0, shapeModel = 0){\n"\
"  # for aggregation = 'monthly' or 'seasonal' analysis, maxes should be a 3-dimensional array of month (1,..,12) by year by location;\n"\
"  # for aggregation = 'annual' analysis, maxes should be a 2-dimensional array of year by location\n"\
"  # aggregation should be either 'annual', 'seasonal', or 'monthly'; if 'annual', 'maxes' should be a two-dimensional array of yearly maxes by location; if 'seasonal' or 'monthly', 'maxes' should be a three-dimensional array of monthly maxes by year and location\n"\
"  # years is an optional argument giving the actual year values of the maxes - this allows some years to be missing\n"\
"  # locationModel, scaleModel, and shapeModel provide the polynomial order of the model for how the location, scale and shape parameters (respectively) vary with time: 0 is no time trend, 1 is linear, 2 quadratic, etc.\n"\
"\n"\
"  # CAUTION for user: parameter values for time-varying models must be interpreted based on transforming numeric years by subtracting the mean year and dividing by the number of years and based on the log transformation of the scale parameter\n"\
"  # we will probably want to write this code so that it provides the user the return values or change in return values for a specified time interval as these are more interpretable than the parameter values\n"\
"  # we will also need to return uncertainty for either the parameters or the return values\n"\
"\n"\
"  require(ismev)\n"\
"\n"\
"  if(aggregation == 'annual'){  # put annual maxes in same 3-d array form as for seasonal/monthly analysis\n"\
"    maxes = array(maxes, c(1, nrow(maxes), ncol(maxes)))\n"\
"  }\n"\
"\n"\
"  nYr = dim(maxes)[2]\n"\
"  nLocs = dim(maxes)[3]\n"\
"  if(is.null(years)){\n"\
"    years = 1:nYr\n"\
"  }\n"\
"\n"\
"  if(aggregation == 'seasonal'){\n"\
"    nSeas = 4\n"\
"    seasonIndices = list(djf = c(1, 2, 12), mam = 3:5, jja = 6:8, son = 9:11)\n"\
"    dec = 12\n"\
"\n"\
"    maxes[dec, 2:nYr, ] = maxes[dec, 1:(nYr - 1), ]\n"\
"    maxes = maxes[ , -1, ] # can't use first year because Dec. is missing, so exclude data for all seasons\n"\
"    nYr = nYr - 1\n"\
"    years = years[-1]\n"\
"\n"\
"    tmpMaxes = array(NA, c(nSeas, nYr, nLocs))\n"\
"    for(j in 1:nSeas){\n"\
"      tmpMaxes[j, , ] = apply(maxes[seasonIndices[[j]], , ], c(2, 3), max, na.rm = TRUE)\n"\
"    }\n"\
"    maxes = tmpMaxes\n"\
"  }\n"\
"\n"\
"  # nStrata = dim(maxes)[1]\n"\
"  \n"\
"  years = (years - mean(years))/nYr  # shift and scale for better numeric properties in estimation\n"\
"\n"\
"\n"\
"\n"\
"  highestOrder = max(c(locationModel, scaleModel, shapeModel))\n"\
"  if(highestOrder){ # any non-zero values\n"\
"    ydat = matrix(years, nc = 1)\n"\
"    cnt = 1\n"\
"    while(cnt < highestOrder){\n"\
"      ydat[ , cnt] = ydat[ , (cnt - 1)] * years\n"\
"      cnt = cnt + 1\n"\
"    }\n"\
"  }\n"\
"\n"\
"  nParam = 3 + locationModel + scaleModel + shapeModel\n"\
"\n"\
"  NAvec = rep(NA, nParam)\n"\
"\n"\
"  mul = NULL; sigl = NULL; shl = NULL\n"\
"\n"\
"  if(locationModel){\n"\
"    mul = 1:locationModel\n"\
"  } else{\n"\
"    mul = NULL\n"\
"  }\n"\
"  mulink = identity\n"\
"\n"\
"  if(scaleModel){\n"\
"    sigl = 1:scaleModel\n"\
"    siglink = exp\n"\
"  } else{\n"\
"    sigl = NULL\n"\
"    siglink = identity\n"\
"  }\n"\
"\n"\
"  if(shapeModel){\n"\
"    shl = 1:shapeModel\n"\
"  } else{\n"\
"    shl = NULL\n"\
"  }\n"\
"  shlink = identity\n"\
"\n"\
"  gev.fit.wrap = function(xdat){\n"\
"    fit = try(gev.fit(xdat, ydat = ydat, mul = mul, sigl = sigl, shl = shl, mulink = mulink, siglink = siglink, shlink = shlink, show = FALSE))\n"\
"    if(!is(fit, 'try-error')){\n"\
"      return(fit$mle)\n"\
"    } else{\n"\
"      return(NAvec)\n"\
"    }\n"\
"  }\n"\
"\n"\
"  output = apply(maxes, c(1, 3), gev.fit.wrap)  # this should be a 3-dim array of dimension nParam by nStrata by nLocs\n"\
"\n"\
"  if(aggregation == 'annual')\n"\
"    output = output[ , 1, ]  # remove dimension with only 1 value (annual analysis)\n"\
"  return(output)\n"\
"}\n"\
    "";
  
  
