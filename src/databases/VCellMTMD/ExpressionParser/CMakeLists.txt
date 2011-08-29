PROJECT(ExpressionParser)    
    
FILE (GLOB SRC_FILES *.cpp)    
    
ADD_LIBRARY(ExpressionParser STATIC ${SRC_FILES})    
