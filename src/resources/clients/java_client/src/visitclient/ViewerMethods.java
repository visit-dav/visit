/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package visitclient;

import com.google.gson.Gson;
import com.google.gson.JsonArray;
import com.google.gson.JsonObject;
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
        JsonObject jo = m_state.data(0);
        
        jo.addProperty(m_state.api(0).get("RPCType").getAsString(), 
                ViewerState.RPCType.InvertBackgroundRPC.ordinal());
        
        m_state.notify(0);
    }
    
    public synchronized void AddWindow()
    {
        JsonObject jo = m_state.data(0);
        
        jo.addProperty(m_state.api(0).get("RPCType").getAsString(), 
                ViewerState.RPCType.AddWindowRPC.ordinal());
        
        m_state.notify(0);
    }
    
    public synchronized void DrawPlots()
    {
        JsonObject jo = m_state.data(0);
        
        jo.addProperty(m_state.api(0).get("RPCType").getAsString(), 
                ViewerState.RPCType.DrawPlotsRPC.ordinal());
        
        m_state.notify(0);
    }
    
    public synchronized void DeleteActivePlots()
    {
        JsonObject jo = m_state.data(0);
        
        jo.addProperty(m_state.api(0).get("RPCType").getAsString(), 
                ViewerState.RPCType.DeleteActivePlotsRPC.ordinal());
        
        m_state.notify(0);
    }

    public synchronized void HideActivePlots()
    {
        JsonObject jo = m_state.data(0);
        
        jo.addProperty(m_state.api(0).get("RPCType").getAsString(), 
                ViewerState.RPCType.HideActivePlotsRPC.ordinal());
        
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
        JsonObject jo = m_state.data(0);
        
        jo.addProperty(m_state.api(0).get("RPCType").getAsString(), 
                ViewerState.RPCType.DeleteActivePlotsRPC.ordinal());
        
        System.out.println( (new Gson()).toJsonTree(index).toString());
        
        JsonArray array = jo.getAsJsonArray(m_state.api(0).get("activePlotIds").getAsString());
        array.add((new Gson()).toJsonTree(index));
        
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
        JsonObject jo = m_state.data(0);
        
        jo.addProperty(m_state.api(0).get("RPCType").getAsString(), 
                ViewerState.RPCType.OpenDatabaseRPC.ordinal());
        
        jo.addProperty(m_state.api(0).get("database").getAsString(), 
                filename);
        
        jo.addProperty(m_state.api(0).get("intArg1").getAsString(), 
                timeState);
        
        jo.addProperty(m_state.api(0).get("boolFlag").getAsString(), 
                addDefaultPlots);

        jo.addProperty(m_state.api(0).get("stringArg1").getAsString(), 
                forcedFileType);        
        m_state.notify(0);
    }
    
    private synchronized int GetEnabledID(String plot_type, String name)
    {
        JsonArray names = m_state.data(14).get(m_state.api(14).get("name").getAsString()).getAsJsonArray();
        JsonArray types = m_state.data(14).get(m_state.api(14).get("type").getAsString()).getAsJsonArray();
        JsonArray enabled = m_state.data(14).get(m_state.api(14).get("enabled").getAsString()).getAsJsonArray();
        
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
        JsonObject jo = m_state.data(0);
        
        jo.addProperty(m_state.api(0).get("RPCType").getAsString(), 
                ViewerState.RPCType.AddPlotRPC.ordinal());
        
        jo.addProperty(m_state.api(0).get("plotType").getAsString(), 
                plot_type);
        
        jo.addProperty(m_state.api(0).get("variable").getAsString(), 
                plot_var);
        
       m_state.notify(0);
    }
    
    public synchronized void AddPlot(String plot_name, String plot_var)
    {
        int index = GetEnabledID("plot", plot_name);
     
        //System.out.println("--> " + index);
        if(index >= 0)
            AddPlotByID(index,plot_var);
    }
    
    private synchronized void AddOperatorByID(int plot_type)
    {
        JsonObject jo = m_state.data(0);
        
        jo.addProperty(m_state.api(0).get("RPCType").getAsString(), 
                ViewerState.RPCType.AddOperatorRPC.ordinal());
        
        jo.addProperty(m_state.api(0).get("operatorType").getAsString(), 
                plot_type);
        
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