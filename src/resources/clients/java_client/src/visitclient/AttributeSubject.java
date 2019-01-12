/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package visitclient;

import java.util.ArrayList;
import java.util.concurrent.ConcurrentHashMap;
import com.google.gson.JsonObject;
import com.google.gson.JsonElement;
import com.google.gson.JsonArray;
import com.google.gson.JsonPrimitive;
import com.google.gson.Gson;

/**
 *
 * @author hari
 */
public class AttributeSubject
{
    public interface AttributeSubjectCallback
    {
        public void update(AttributeSubject subject);
    };
        
    public AttributeSubject()
    {
        update = new Updater();
        callbackList = new ArrayList<AttributeSubjectCallback>();
    }

    public boolean update(JsonObject jo)
    {
        if(jo.has("api")) {
            api = jo;
            update.id = api.get("id").getAsInt();
        }
        else
        {
            data = jo;
            // tell all listeners object has been updated..
            for(AttributeSubjectCallback cb : callbackList) cb.update(this);
        }
        return true;
    }

    private JsonArray getData()
    {
        return data.getAsJsonArray("contents"); //.getAsJsonObject("data");
    }
    
    private JsonObject getApi()
    {
        return api.getAsJsonObject("api"); //.getAsJsonObject("data");
    }
    
    private boolean typeEquals(JsonElement a, JsonElement b)
    {
        if(a.isJsonNull() && b.isJsonNull() ||
           a.isJsonArray() && b.isJsonArray() ||
           a.isJsonArray() && b.isJsonArray())
            return true;
        
        if(a.isJsonPrimitive() && b.isJsonPrimitive())
        {
            JsonPrimitive ap = a.getAsJsonPrimitive();
            JsonPrimitive bp = b.getAsJsonPrimitive();
            if((ap.isBoolean() && bp.isBoolean()) ||
               (ap.isNumber() && bp.isNumber()) ||
               (ap.isString() && bp.isString()))
                return true;
        }
        
        return false;
    }
    
    public void set(String key, JsonElement value) {
        
        int index = getApi().get(key).getAsInt();
        JsonElement p = getData().get(index);
        
        /// check if they are the same type..
        if(!typeEquals(p, value)) {
            System.err.println("Types do not equal each other..");
            return;
        }
        
        /// add to mod list..
        update.insert(index, value, data.getAsJsonArray("metadata").get(index));
        
    }
    
    public JsonElement get(String key) 
    {
        int index = getApi().get(key).getAsInt();
        if(update.contains(index)) {
            return update.get(index);
        }
        return getData().get(index);
    }
    
    public void notify(java.io.OutputStreamWriter output)
    {
        try
        {
            Gson gson = new Gson();
            String result = gson.toJson(update, Updater.class);
            update.clear();
            
            output.write(result);
            output.flush();
            
            
        }
        catch(Exception e)
        {
            System.out.println("Unable to write data to VisIt");
        }
    }
    
    public void addCallback(AttributeSubjectCallback callback)
    {
        callbackList.add(callback);
    }
    
    
    public void removeCallback(AttributeSubjectCallback callback)
    {
        callbackList.remove(callback);
    }
    
    private class Updater {
        
        public Integer id;
        public ConcurrentHashMap<Integer, JsonElement> contents;
        public ConcurrentHashMap<Integer, JsonElement> metadata;
    
        Updater() {
            contents = new ConcurrentHashMap<Integer, JsonElement>();
            metadata = new ConcurrentHashMap<Integer, JsonElement>();
        }
        void clear() {
            contents.clear();
            metadata.clear();
        }
        
        void insert(int index, JsonElement d, JsonElement md) {
            contents.put(index, d);
            metadata.put(index, md);
        }
        
        boolean contains(int index) {
            return contents.contains(index);
        }
        
        JsonElement get(int index) {
            return contents.get(index);
        }
    }
    
    private JsonObject data;
    private JsonObject api;
    private Updater update;
    public ArrayList<AttributeSubjectCallback> callbackList;
}
    
