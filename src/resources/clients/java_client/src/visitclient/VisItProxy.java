/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package visitclient;

import com.google.gson.Gson;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.ConnectException;
import java.net.Socket;

/**
 *
 * @author hari
 */
public class VisItProxy
{

    public static final int BUFSIZE = 4096;
    
    class Header
    {
        public String password;
        public String canRender;
    }
        
    class VisItThread implements Runnable
    {
        private InputStreamReader inputConnection;
        private Gson gson;
        
        VisItThread(InputStreamReader i)
        {
            gson = new Gson();
            inputConnection = i;
        }
        
        private int count(String str, String findStr)
        {
            int lastIndex = 0;
            int count =0;

            while(lastIndex != -1)
            {

                lastIndex = str.indexOf(findStr,lastIndex);

                if( lastIndex != -1){
                    count ++;
                    lastIndex+=findStr.length();
                }
            }
            
            return count;
        }
        
        @Override
        public void run()
        {
            StringBuilder partial_entry = new StringBuilder(""); //stitches together one map_node entry
            StringBuilder input_buffer = new StringBuilder(""); //holds input data buffer

            char [] data = new char [ VisItProxy.BUFSIZE ];
            
            while(true)
            {
                int len = 0;
                try {
                    len = inputConnection.read(data);
                }
                catch(Exception e) 
                {
                    System.out.println("Exception: Quitting ...");
                    break;
                }
                
                if(len == 0)
                {
                    System.out.println("Quitting ...");
                    break;
                }
                
                input_buffer.append(data,0,len);
                
                // for now JSON parser has to start with object..
                int mnsi = input_buffer.indexOf("{");
                int mnei = input_buffer.indexOf("}");

                while (mnsi >= 0 || mnei >= 0)
                {
                    if (mnsi < 0 && mnei >= 0)
                    {
                        mnei += "}".length();
                        partial_entry.append(input_buffer.subSequence(0,mnei));
                        input_buffer.delete(0, mnei);
                    }
                    else if (mnsi >= 0 && mnei < 0)
                    {
                        mnsi += "{".length();
                        partial_entry.append(input_buffer.subSequence(0,mnsi));
                        input_buffer.delete(0, mnsi);
                    }
                    else
                    {
                        if( mnsi < mnei )
                        {
                            mnsi += "{".length();
                            partial_entry.append(input_buffer.subSequence(0, mnsi));
                            input_buffer.delete(0,mnsi);
                        }
                        else
                        {
                            mnei += "}".length();
                            partial_entry.append(input_buffer.subSequence(0,mnei));
                            input_buffer.delete(0,mnei);
                        }
                    }
                
                    String tmp = partial_entry.toString().trim();

                    if (count(tmp,"{") > 0 && count(tmp,"{") == count(tmp,"}"))
                    {
                        try {
                            partial_entry.setLength(0);
                            //tmp=tmp.replace("\n","");
                            //tmp=tmp.replace("\\\"","");
                            JsonElement el = gson.fromJson(tmp,JsonElement.class);
                            JsonObject jo = el.getAsJsonObject();

                            ///update state..
                            VisItProxy.this.state.update(jo);
                        }
                        catch(Exception e)
                        {
                            System.out.println("failed input " + tmp);
                        }
                    }

                    //tmp = input_buffer.strip()

                    //print partial_entry
                    mnsi = input_buffer.indexOf("{"); //don't include current node
                    mnei = input_buffer.indexOf("}");
                }
            }
        }
    }
    
    private boolean handshake(String host, int port, String password)
    {
        try {
            Gson gson = new Gson();
            
            Socket socket = new Socket(host,port);
            OutputStreamWriter writer = new OutputStreamWriter(socket.getOutputStream());
            InputStreamReader reader = new InputStreamReader(socket.getInputStream());
           
            Header header = new Header();
            header.password = password;
            header.canRender = "data";
            
            String headerstr = gson.toJson(header);
//            System.out.println("Writing message: " + headerstr);
            
            writer.write(headerstr);
            writer.flush();
            
//            System.out.println("Reading...");
            char [] cbuf = new char [ 1024 ];
            int len = reader.read(cbuf);
            String message = new String(cbuf,0,len);
           
            JsonElement e = gson.fromJson(message,JsonElement.class);
            JsonObject jo = e.getAsJsonObject();
 
            visit_host = jo.get("host").getAsString();
            visit_port = jo.get("port").getAsString();
            visit_security_key = jo.get("securityKey").getAsString();
            
            //System.out.println(visit_host + " " + visit_port + " " + visit_security_key);
            socket.close();
            
            return true;
        }
        catch(ConnectException e)
        {
            System.out.println("Exception: Could not connect to a running VisIt client");
        }
        catch(Exception e)
        {
            System.out.println("Exception");
        }
        return false;
    }
    
    public boolean connect(String host, int port, String password)
    {
        if(!handshake(host, port, password))
            return false;
        
        try{
            
            inputSocket = new Socket(visit_host,Integer.valueOf(visit_port));
            inputConnection = new InputStreamReader(inputSocket.getInputStream());
            
            outputSocket = new Socket(visit_host,Integer.valueOf(visit_port));
            outputConnection = new OutputStreamWriter(outputSocket.getOutputStream());
            
            
            // Handle initial connection 
            char [] cbuf = new char [1024];
            
            //read 100 bytes
            InputStreamReader isr = (new InputStreamReader(outputSocket.getInputStream()));
            isr.read(cbuf);
            
            //extract socket key
            //visit_socket_key = new String(cbuf,5+1+10+21, 21);
            
            cbuf[0] = ASCIIFORMAT;
            
            for(int i = 0; i < visit_security_key.length(); ++i)
                cbuf[6+10+i] = visit_security_key.charAt(i);
            
            OutputStreamWriter osw = new OutputStreamWriter(inputSocket.getOutputStream());
            osw.write(cbuf);
            osw.flush();
        
            //System.out.println("wrote: " + new String(cbuf));
            // End - Handle initial connection
            
            state.setConnection(outputConnection);
            
            thread = new Thread(new VisItThread(inputConnection));
            
            thread.setDaemon(true);
            thread.start();
            
            //Hackish sync state..
            while(state.states.size() < 135)
            {
                try{Thread.sleep(100); }catch(Exception e) {}
            }

            System.out.println("Viewer State synched..");

        }
        catch(Exception e)
        {
            System.out.println("Exception connecting..");
        }
        return true;
    }
    
    public VisItProxy()
    {
        state = new ViewerState();
        methods = new ViewerMethods(state);
    }
    
    public ViewerState GetViewerState()
    {
        return state;
    }
    
    public ViewerMethods GetViewerMethods()
    {
        return methods;
    }
    
    private String visit_host, visit_port;
    //private String visit_socket_key;
    private String visit_security_key;
    private InputStreamReader inputConnection;
    private OutputStreamWriter outputConnection;
    private Socket inputSocket, outputSocket;
    private Thread thread;
   
    private static final byte ASCIIFORMAT = 0;
    //private static final byte BINARYFORMAT = 1;

    private ViewerState state;
    private ViewerMethods methods;
}
