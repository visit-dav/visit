/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package visitclient;

import com.google.gson.Gson;
import com.google.gson.JsonArray;
import java.util.ArrayList;

/**
 *
 * @author hari
 */
public class ViewerMethods
{
    ViewerState m_state;

    public ViewerMethods(ViewerState state)
    {
        m_state = state;
    }
    
    public synchronized void InvertBackgroundColor()
    {
        m_state.set(0, "RPCType", ViewerState.RPCType.InvertBackgroundRPC.ordinal());
        m_state.notify(0);
    }
    
    public synchronized void AddWindow()
    {
        m_state.set(0, "RPCType", ViewerState.RPCType.AddWindowRPC.ordinal());
        m_state.notify(0);
    }
    
    public synchronized void DrawPlots()
    {
        m_state.set(0, "RPCType", ViewerState.RPCType.DrawPlotsRPC.ordinal());
        m_state.notify(0);
    }
    
    public synchronized void DeleteActivePlots()
    {
        m_state.set(0, "RPCType", ViewerState.RPCType.DeleteActivePlotsRPC.ordinal());
        m_state.notify(0);
    }

    public synchronized void HideActivePlots()
    {
        m_state.set(0, "RPCType", ViewerState.RPCType.HideActivePlotsRPC.ordinal());
        m_state.notify(0);
    }
    
    public synchronized void SetActivePlots(int index)
    {
        ArrayList<Integer> list = new ArrayList<Integer>();
        list.add(index);
        
        SetActivePlots(list);
    }
    
    public synchronized void SetActivePlots(ArrayList<Integer> index)
    {   
        m_state.set(0, "RPCType", ViewerState.RPCType.SetActivePlotsRPC.ordinal());
        m_state.set(0, "activePlotIds", (new Gson()).toJsonTree(index));
        m_state.notify(0);
    }
        
    public synchronized void OpenDatabase(String filename)
    {
        OpenDatabase(filename,0,true,"");
    }
    
    public synchronized void OpenDatabase(String filename, 
                                          int timeState)
    {
        OpenDatabase(filename,timeState,true,"");
    }
    
    public synchronized void OpenDatabase(String filename, 
                                          int timeState, 
                                          boolean addDefaultPlots)
    {
        OpenDatabase(filename,timeState,addDefaultPlots,"");
    }
    
    public synchronized void OpenDatabase(String filename, 
                                          int timeState, 
                                          boolean addDefaultPlots, 
                                          String forcedFileType)
    {
        m_state.set(0, "RPCType", ViewerState.RPCType.OpenDatabaseRPC.ordinal());
        m_state.set(0, "database", filename);
        m_state.set(0, "intArg1", timeState);
        m_state.set(0, "boolFlag", addDefaultPlots);
        m_state.set(0, "stringArg1", forcedFileType);
        
        m_state.notify(0);
    }
    
    private synchronized int GetEnabledID(String plot_type, String name)
    {
        JsonArray names = m_state.get(14, "name").getAsJsonArray();
        JsonArray types = m_state.get(14, "type").getAsJsonArray();
        JsonArray enabled = m_state.get(14, "enabled").getAsJsonArray();
        
        ArrayList<String> mapper = new ArrayList<String>();
        
        //System.out.println(names);
        //System.out.println(types);
        //System.out.println(enabled);
        
        for(int i = 0; i < names.size(); ++i)
        {
            //System.out.println(enabled.get(i).getAsInt() + " " 
            //                   + types.get(i).getAsString());
            if(enabled.get(i).getAsInt() == 1 && 
               plot_type.equals(types.get(i).getAsString()))
                    mapper.add(names.get(i).getAsString());
        }
        
        java.util.Collections.sort(mapper);
        
        //the JSON map contains strings with quotes..
        //if(!name.contains("\"")) name = "\"" + name + "\"";
        
        for(int i = 0; i < mapper.size(); ++i)
        {
            //System.out.println(name + " " + mapper.get(i));
            if(name.equals(mapper.get(i))) return i;
        }
        return -1;
    }
    
    private synchronized void AddPlotByID(int plot_type, String plot_var)
    {
       m_state.set(0, "RPCType", ViewerState.RPCType.AddPlotRPC.ordinal());
       m_state.set(0, "plotType", plot_type);
       m_state.set(0, "variable", plot_var);
       m_state.notify(0);
    }
    
    public synchronized void AddPlot(String plot_name, String plot_var)
    {
        int index = GetEnabledID("plot", plot_name);
     
        //System.out.println("--> " + index);
        if(index >= 0)
            AddPlotByID(index,plot_var);
    }
    
    private synchronized void AddOperatorByID(int op_type)
    {
        m_state.set(0, "RPCType", ViewerState.RPCType.AddOperatorRPC.ordinal());
        m_state.set(0, "operatorType", op_type);
        m_state.notify(0);
    }
    
    public synchronized void AddOperator(String operator_name)
    {
        int index = GetEnabledID("operator", operator_name);
        
        //System.out.println("--> " + index);
        if(index >= 0)
            AddOperatorByID(index);
    }
}
