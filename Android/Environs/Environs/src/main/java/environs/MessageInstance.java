package environs;
/**
 *	MessageInstance
 * ------------------------------------------------------------------
 * Copyright (c) Chi-Tai Dang
 *
 * @author	Chi-Tai Dang
 * @version	1.0
 * @remarks
 *
 * This file is part of the Environs framework developed at the
 * Lab for Human Centered Multimedia of the University of Augsburg.
 * http://hcm-lab.de/environs
 *
 * Environ is free software; you can redistribute it and/or modify
 * it under the terms of the Eclipse Public License v1.0.
 * A copy of the license may be obtained at:
 * http://www.eclipse.org/org/documents/epl-v10.html
 * --------------------------------------------------------------------
 */

/**
 *	MessageInstance
 *	---------------------------------------------------------
 *	Copyright (C) Chi-Tai Dang
 *   All rights reserved.
 *
 *	@author		Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *	@version	1.0
 * ****************************************************************************************
 */
@SuppressWarnings ( "unused" )
public class MessageInstance
{
    private static final String className = "MessageInstance. . . . .";

    /**
     * sent is true if this MessageInstance is data that was sent or received (false).
     * */
    public boolean sent = false;

    /**
     * created is a posix timestamp that determines the time and date that this MessageInstance
     * has been received or sent.
     * */
    public long created;

    /**
     * The length of the text message in bytes (characters).
     * */
    public int length = 0;

    /**
     * The text message.
     * */
    public String text;

    /**
     * Determins the type of connection (channel type) used to exchange this message.
     * c = in connected state
     * d = in not connected state through a direct device to device channel.
     * m = in not connected state by means of a Mediator service.
     * */
    public char connection = 'c';

    /**
     * A reference to the DeviceInstance that is responsible for this FileInstance.
     * */
    public DeviceInstance device = null;


    static MessageInstance Init(String line, DeviceInstance _device)
    {
        if (line == null || line.length() < 13 || _device == null)
            return null;

        int pos = line.indexOf(' ');
        if (pos <= 0 || (pos + 1 > line.length()) )
            return null;

        String unixTime = line.substring(3, pos);
        long unixLong = Long.parseLong(unixTime);

        MessageInstance msg = new MessageInstance();
        msg.sent = (line.charAt(0) == 'o');
        msg.connection = line.charAt(1);

        msg.device = _device;
        msg.created = unixLong;

        msg.text = line.substring(pos + 1);
        //Utils.Log(1, className, "Init: Added new message. " + msg.toString());
        return msg;
    }


    @SuppressWarnings ( "SimplifiableIfStatement" )
    static boolean HasPrefix (String line)
    {
        if (line.length() < 13 || (line.charAt(0) != 'i' && line.charAt(0) != 'o') )
            return false;

        return line.charAt ( 2 ) == ':';
    }


    public String ShortText()
    {
        if (text == null) return "";
        if (text.length() < 260) return text;
        return text.substring(0, 260);
    }

    @Override
    public String toString()
    {
        return "MessageInstance " + (sent ? "sent" : "received") + " on [" + new java.util.Date(created*1000) + "]  text ["
                + ShortText()  + "]";
    }

    public String ToString() {
        return toString();
    }
}
