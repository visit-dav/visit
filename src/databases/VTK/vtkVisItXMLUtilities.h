/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVisItXMLUtilities.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkVisItXMLUtilities - XML utilities.
// .SECTION Description
// vtkVisItXMLUtilities provides XML-related convenience functions.
// .SECTION See Also
// vtkVisItXMLDataElement

#ifndef __vtkVisItXMLUtilities_h
#define __vtkVisItXMLUtilities_h

#include "vtkObject.h"

class vtkVisItXMLDataElement;

class vtkVisItXMLUtilities : public vtkObject
{
public:
  static vtkVisItXMLUtilities* New();
  vtkTypeRevisionMacro(vtkVisItXMLUtilities, vtkObject);

  // Description:
  // Encode a string from one format to another 
  // (see VTK_ENCODING_... constants).
  // If special_entites is true, convert some characters to their corresponding
  // character entities.
  static void EncodeString(const char *input, int input_encoding, 
                           ostream &output, int output_encoding,
                           int special_entities = 0);

  // Description:
  // Collate a vtkVisItXMLDataElement's attributes to a stream as a series of
  // name="value" pairs (the separator between each pair can be specified,
  // if not, it defaults to a space). 
  // Note that the resulting character-encoding will be UTF-8 (we assume
  // that this function is used to create XML files/streams).
  static void CollateAttributes(vtkVisItXMLDataElement*, 
                                ostream&, 
                                const char *sep = 0);

  //BTX
  // Description:
  // Flatten a vtkVisItXMLDataElement to a stream, i.e. output a textual stream
  // corresponding to that XML element, its attributes and its
  // nested elements.
  // If 'indent' is not NULL, it is used to indent the whole tree.
  // If 'indent' is not NULL and 'indent_attributes' is true, attributes will 
  // be indented as well.
  // Note that the resulting character-encoding will be UTF-8 (we assume
  // that this function is used to create XML files/streams).
  static void FlattenElement(vtkVisItXMLDataElement*, 
                             ostream&, 
                             vtkIndent *indent = 0,
                             int indent_attributes = 1);

  // Description:
  // Write a vtkVisItXMLDataElement to a file (in a flattened textual form)
  // Note that the resulting character-encoding will be UTF-8.
  // Return 1 on success, 0 otherwise.
  static int WriteElementToFile(vtkVisItXMLDataElement*, 
                                const char *filename, 
                                vtkIndent *indent = 0);
  //ETX

  // Description:
  // Read a vtkVisItXMLDataElement from a stream, string or file.
  // The 'encoding' parameter will be used to set the internal encoding of the
  // attributes of the data elements created by those functions (conversion
  // from the XML stream encoding to that new encoding will be performed
  // automatically). If set to VTK_ENCODING_NONE, the encoding won't be
  // changed and will default to the default vtkVisItXMLDataElement encoding.
  // Return the root element on success, NULL otherwise.
  // Note that you have to call Delete() on the element returned by that
  // function to ensure it is freed properly.
  //BTX
  static vtkVisItXMLDataElement* ReadElementFromStream(
    istream&, int encoding = VTK_ENCODING_NONE);
  static vtkVisItXMLDataElement* ReadElementFromString(
    const char *str, int encoding = VTK_ENCODING_NONE);
  static vtkVisItXMLDataElement* ReadElementFromFile(
    const char *filename, int encoding = VTK_ENCODING_NONE);
  //ETX

  // Description:
  // Find all elements in 'tree' that are similar to 'elem' (using the
  // vtkVisItXMLDataElement::IsEqualTo() predicate). 
  // Return the number of elements found and store those elements in
  // 'results' (automatically allocated).
  // Warning: the results do not include 'elem' if it was found in the tree ;
  // do not forget to deallocate 'results' if something was found.
  //BTX
  static int FindSimilarElements(vtkVisItXMLDataElement *elem, 
                                 vtkVisItXMLDataElement *tree, 
                                 vtkVisItXMLDataElement ***results);
  //ETX

  // Description:
  // Factor and unfactor a tree. This operation looks for duplicate elements
  // in the tree, and replace them with references to a pool of elements.
  // Unfactoring a non-factored element is harmless.
  static void FactorElements(vtkVisItXMLDataElement *tree);
  static void UnFactorElements(vtkVisItXMLDataElement *tree);

protected:  
  vtkVisItXMLUtilities() {};
  ~vtkVisItXMLUtilities() {};
  
  static int FactorElementsInternal(vtkVisItXMLDataElement *tree, 
                                    vtkVisItXMLDataElement *root, 
                                    vtkVisItXMLDataElement *pool);
  static int UnFactorElementsInternal(vtkVisItXMLDataElement *tree, 
                                      vtkVisItXMLDataElement *pool);

private:
  vtkVisItXMLUtilities(const vtkVisItXMLUtilities&); // Not implemented
  void operator=(const vtkVisItXMLUtilities&); // Not implemented    
};

#endif


