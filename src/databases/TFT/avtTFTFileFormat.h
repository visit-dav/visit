
// ************************************************************************* //
//                            avtTFTFileFormat.h                             //
// ************************************************************************* //

#ifndef AVT_TFT_FILE_FORMAT_H
#define AVT_TFT_FILE_FORMAT_H
#include <avtSTSDFileFormat.h>
#include <visitstream.h>
#include <string>
#include <map>

// ****************************************************************************
//  Class: avtTFTFileFormat
//
//  Purpose:
//      Reads in TFT files as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Feb 22 16:04:15 PST 2005
//
//  Modifications:
//
// ****************************************************************************

class avtTFTFileFormat : public avtSTSDFileFormat
{
public:
                           avtTFTFileFormat(const char *filename);
    virtual               ~avtTFTFileFormat();

    //
    // These are used to declare what the current time and cycle are for the
    // file.  These should only be defined if the file format knows what the
    // time and/or cycle is.
    //
    virtual bool           ReturnsValidTime() const;
    virtual double         GetTime(void);

    virtual const char    *GetType(void)   { return "TFT"; };
    virtual void           FreeUpResources(void); 
    virtual void           ActivateTimestep(void);

    virtual vtkDataSet    *GetMesh(const char *);
    virtual vtkDataArray  *GetVar(const char *);

protected:
    void                   Initialize();
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);

    class CurveData
    {
    public:
                     CurveData();
        virtual     ~CurveData();
        bool         Read(ifstream &, bool &, float &);
        std::string  GetLine(ifstream &);
        std::string  StripExcess(const std::string &input);
        void         GetLabelsAndUnits(const std::string &, std::string &,
                                       std::string &);

        std::string  title;
        std::string  subtitle;
        std::string  xLabel;
        std::string  xUnits;
        std::string  yLabel;
        std::string  yUnits;
        int          nPoints;
        float       *data;
        float        xmin;
        float        xmax;
        float        ymin;
        float        ymax;
    };

    typedef std::map<std::string, CurveData *> CurveDataMap;

    CurveDataMap     variables;
    bool             initialized;
    bool             hasValidTime;
    float            time;
};


#endif
