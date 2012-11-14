/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package visitclient;

import com.google.gson.JsonObject;

/**
 *
 * @author hari
 */
public class AttributeSubject
{
    public AttributeSubject()
    {}

    public boolean update(JsonObject jo)
    {
        if(jo.has("api"))
            api = jo;
        else
            data = jo;
        return true;
    }

    public JsonObject getData()
    {
        return data.getAsJsonObject("contents").getAsJsonObject("data");
    }
    
    public JsonObject getApi()
    {
        return api.getAsJsonObject("api").getAsJsonObject("data");
    }
    
    public JsonObject data;
    public JsonObject api;
}
    
