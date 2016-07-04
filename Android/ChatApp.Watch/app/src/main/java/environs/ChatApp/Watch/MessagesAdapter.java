package environs.ChatApp.Watch;
/**
 *	MessagesAdapter
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
import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

import java.util.ArrayList;
import environs.*;
import environs.ChatApp.Watch.R;

/**
 * MessagesAdapter realizes a BaseAdapter to present the chat history of the current active chat session.
 *
 * @author Chi-Tai Dang, dang@hcm-lab.de, University of Augsburg
 *
 */
public class MessagesAdapter extends BaseAdapter
{
    public ArrayList<MessageInstance> messages;
    public Context context;
    LayoutInflater inflater;

    public MessagesAdapter(Context context, ArrayList<MessageInstance> messages) {
        this.messages = messages;
        this.context = context;
        inflater = LayoutInflater.from(this.context);
    }

    @Override
    public int getCount() {
        return messages.size();
    }

    @Override
    public MessageInstance getItem(int position) {
        return messages.get(position);
    }

    @Override
    public long getItemId(int position) {
        return 0;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        if (convertView == null) {
            convertView = inflater.inflate( R.layout.messages_list_item, parent, false);
        }

        MessageInstance msgInst = getItem(position);


        TextView tv = (TextView) convertView.findViewById( R.id.messageText1);
        if (tv != null) {

            if (!msgInst.sent) {
                tv.setText("@" + (MessagesActivity.chatUser != null ? MessagesActivity.chatUser.userName : "Unknown") + ": " + msgInst.text);
            }
            else
                tv.setText(msgInst.text);
        }
        return convertView;
    }
}
