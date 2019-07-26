// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

package llnl.visit;


// ****************************************************************************
// Class: avtSimulationCommandSpecification
//
// Purpose:
//    Contains the specification for one command
//
// Notes:      Autogenerated by xml2java.
//
// Programmer: xml2java
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

public class avtSimulationCommandSpecification extends AttributeSubject
{
    private static int avtSimulationCommandSpecification_numAdditionalAtts = 10;

    // Enum values
    public final static int RUNMODE_UNKNOWN = 0;
    public final static int RUNMODE_RUNNING = 1;
    public final static int RUNMODE_STOPPED = 2;

    public final static int COMMANDARGUMENTTYPE_CMDARGNONE = 0;
    public final static int COMMANDARGUMENTTYPE_CMDARGINT = 1;
    public final static int COMMANDARGUMENTTYPE_CMDARGFLOAT = 2;
    public final static int COMMANDARGUMENTTYPE_CMDARGSTRING = 3;


    public avtSimulationCommandSpecification()
    {
        super(avtSimulationCommandSpecification_numAdditionalAtts);

        name = new String("");
        argumentType = COMMANDARGUMENTTYPE_CMDARGNONE;
        className = new String("");
        enabled = true;
        parent = new String("");
        isOn = true;
        signal = new String("");
        text = new String("");
        uiType = new String("");
        value = new String("");
    }

    public avtSimulationCommandSpecification(int nMoreFields)
    {
        super(avtSimulationCommandSpecification_numAdditionalAtts + nMoreFields);

        name = new String("");
        argumentType = COMMANDARGUMENTTYPE_CMDARGNONE;
        className = new String("");
        enabled = true;
        parent = new String("");
        isOn = true;
        signal = new String("");
        text = new String("");
        uiType = new String("");
        value = new String("");
    }

    public avtSimulationCommandSpecification(avtSimulationCommandSpecification obj)
    {
        super(obj);

        name = new String(obj.name);
        argumentType = obj.argumentType;
        className = new String(obj.className);
        enabled = obj.enabled;
        parent = new String(obj.parent);
        isOn = obj.isOn;
        signal = new String(obj.signal);
        text = new String(obj.text);
        uiType = new String(obj.uiType);
        value = new String(obj.value);

        SelectAll();
    }

    public int Offset()
    {
        return super.Offset() + super.GetNumAdditionalAttributes();
    }

    public int GetNumAdditionalAttributes()
    {
        return avtSimulationCommandSpecification_numAdditionalAtts;
    }

    public boolean equals(avtSimulationCommandSpecification obj)
    {
        // Create the return value
        return ((name.equals(obj.name)) &&
                (argumentType == obj.argumentType) &&
                (className.equals(obj.className)) &&
                (enabled == obj.enabled) &&
                (parent.equals(obj.parent)) &&
                (isOn == obj.isOn) &&
                (signal.equals(obj.signal)) &&
                (text.equals(obj.text)) &&
                (uiType.equals(obj.uiType)) &&
                (value.equals(obj.value)));
    }

    // Property setting methods
    public void SetName(String name_)
    {
        name = name_;
        Select(0);
    }

    public void SetArgumentType(int argumentType_)
    {
        argumentType = argumentType_;
        Select(1);
    }

    public void SetClassName(String className_)
    {
        className = className_;
        Select(2);
    }

    public void SetEnabled(boolean enabled_)
    {
        enabled = enabled_;
        Select(3);
    }

    public void SetParent(String parent_)
    {
        parent = parent_;
        Select(4);
    }

    public void SetIsOn(boolean isOn_)
    {
        isOn = isOn_;
        Select(5);
    }

    public void SetSignal(String signal_)
    {
        signal = signal_;
        Select(6);
    }

    public void SetText(String text_)
    {
        text = text_;
        Select(7);
    }

    public void SetUiType(String uiType_)
    {
        uiType = uiType_;
        Select(8);
    }

    public void SetValue(String value_)
    {
        value = value_;
        Select(9);
    }

    // Property getting methods
    public String  GetName() { return name; }
    public int     GetArgumentType() { return argumentType; }
    public String  GetClassName() { return className; }
    public boolean GetEnabled() { return enabled; }
    public String  GetParent() { return parent; }
    public boolean GetIsOn() { return isOn; }
    public String  GetSignal() { return signal; }
    public String  GetText() { return text; }
    public String  GetUiType() { return uiType; }
    public String  GetValue() { return value; }

    // Write and read methods.
    public void WriteAtts(CommunicationBuffer buf)
    {
        if(WriteSelect(0, buf))
            buf.WriteString(name);
        if(WriteSelect(1, buf))
            buf.WriteInt(argumentType);
        if(WriteSelect(2, buf))
            buf.WriteString(className);
        if(WriteSelect(3, buf))
            buf.WriteBool(enabled);
        if(WriteSelect(4, buf))
            buf.WriteString(parent);
        if(WriteSelect(5, buf))
            buf.WriteBool(isOn);
        if(WriteSelect(6, buf))
            buf.WriteString(signal);
        if(WriteSelect(7, buf))
            buf.WriteString(text);
        if(WriteSelect(8, buf))
            buf.WriteString(uiType);
        if(WriteSelect(9, buf))
            buf.WriteString(value);
    }

    public void ReadAtts(int index, CommunicationBuffer buf)
    {
        switch(index)
        {
        case 0:
            SetName(buf.ReadString());
            break;
        case 1:
            SetArgumentType(buf.ReadInt());
            break;
        case 2:
            SetClassName(buf.ReadString());
            break;
        case 3:
            SetEnabled(buf.ReadBool());
            break;
        case 4:
            SetParent(buf.ReadString());
            break;
        case 5:
            SetIsOn(buf.ReadBool());
            break;
        case 6:
            SetSignal(buf.ReadString());
            break;
        case 7:
            SetText(buf.ReadString());
            break;
        case 8:
            SetUiType(buf.ReadString());
            break;
        case 9:
            SetValue(buf.ReadString());
            break;
        }
    }

    public String toString(String indent)
    {
        String str = new String();
        str = str + stringToString("name", name, indent) + "\n";
        str = str + indent + "argumentType = ";
        if(argumentType == COMMANDARGUMENTTYPE_CMDARGNONE)
            str = str + "COMMANDARGUMENTTYPE_CMDARGNONE";
        if(argumentType == COMMANDARGUMENTTYPE_CMDARGINT)
            str = str + "COMMANDARGUMENTTYPE_CMDARGINT";
        if(argumentType == COMMANDARGUMENTTYPE_CMDARGFLOAT)
            str = str + "COMMANDARGUMENTTYPE_CMDARGFLOAT";
        if(argumentType == COMMANDARGUMENTTYPE_CMDARGSTRING)
            str = str + "COMMANDARGUMENTTYPE_CMDARGSTRING";
        str = str + "\n";
        str = str + stringToString("className", className, indent) + "\n";
        str = str + boolToString("enabled", enabled, indent) + "\n";
        str = str + stringToString("parent", parent, indent) + "\n";
        str = str + boolToString("isOn", isOn, indent) + "\n";
        str = str + stringToString("signal", signal, indent) + "\n";
        str = str + stringToString("text", text, indent) + "\n";
        str = str + stringToString("uiType", uiType, indent) + "\n";
        str = str + stringToString("value", value, indent) + "\n";
        return str;
    }


    // Attributes
    private String  name;
    private int     argumentType;
    private String  className;
    private boolean enabled;
    private String  parent;
    private boolean isOn;
    private String  signal;
    private String  text;
    private String  uiType;
    private String  value;
}

