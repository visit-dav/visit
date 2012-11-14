/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package visitclient;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

/**
 *
 * @author hari
 */
public class VisItClient 
{

    /**
     * @param args the command line arguments
     */
    public static void main(String [] args) throws InterruptedException, IOException
    {
        System.out.println("Starting Java Client");
        
        VisItProxy client = new VisItProxy();
        
        if(!client.connect("localhost",9002,"bob"))
        {
            System.out.println("Could not connect to VisIt, Quitting");
            System.exit(0);
        }
        
        String CurLine = ""; // Line read from standard in
        
        System.out.println("Enter a line of text (type 'quit' to exit): ");
        InputStreamReader converter = new InputStreamReader(System.in);
        BufferedReader in = new BufferedReader(converter);
  
        ViewerMethods methods = client.GetViewerMethods();
        
        while (!(CurLine.equals("quit()")))
        {
            System.out.print(">> ");
            CurLine = in.readLine().trim();
              
            try{
                if ((CurLine.equals("quit()")))
                    continue;
                
                if("InvertBackgroundColor()".equals(CurLine))
                    methods.InvertBackgroundColor();

                if("AddWindow()".equals(CurLine))
                    methods.AddWindow();

                if("DrawPlots()".equals(CurLine))
                    methods.DrawPlots();

                if(CurLine.startsWith("OpenDatabase"))
                {
                    String ss = CurLine.substring(CurLine.indexOf("\"")+1,CurLine.lastIndexOf("\""));
                    methods.OpenDatabase(ss);
                }
                if(CurLine.startsWith("AddPlot"))
                {
                    String is = CurLine.substring(CurLine.indexOf("(")+1,CurLine.indexOf(")"));
                    is = is.replace("\"", "");

                    String [] results = is.split(",");
                    if(results.length == 2)
                        methods.AddPlot(results[0], results[1]);
                }

                if(CurLine.startsWith("AddOperator"))
                {
                    String is = CurLine.substring(CurLine.indexOf("(")+1,CurLine.indexOf(")"));
                    is = is.replace("\"", "");
                    methods.AddOperator(is);
                }

                if(CurLine.startsWith("DeleteActivePlots()"))
                    methods.DeleteActivePlots();

                if(CurLine.startsWith("HideActivePlots()"))
                    methods.HideActivePlots();

                if(CurLine.startsWith("SetActivePlots"))
                {
                    String is = CurLine.substring(CurLine.indexOf("(")+1,CurLine.indexOf(")"));
                    is = is.replace("\"", "");
                    methods.SetActivePlots(Integer.parseInt(is));
                }
            }
            catch(Exception e)
            {
                System.out.println("Error evaluating: " + CurLine);
            }
        }
        
        System.exit(0);
    }
}
