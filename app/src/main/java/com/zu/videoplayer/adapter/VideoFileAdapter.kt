package com.zu.videoplayer.adapter

import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import androidx.recyclerview.widget.RecyclerView
import com.zu.videoplayer.R
import com.zu.videoplayer.bean.VideoBean

class VideoFileAdapter: RecyclerView.Adapter<VideoFileViewHolder>()
{

    var data: ArrayList<VideoBean>? = null
        set(value) {
            field = value
            notifyDataSetChanged()
        }

    var itemClickListener: ((position: Int) -> Unit)? = null

    var internalItemClickerListener = object : View.OnClickListener{
        override fun onClick(v: View?) {
            if (v == null)
            {
                return
            }
            itemClickListener?.invoke(v!!.tag as Int)
        }
    }
    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): VideoFileViewHolder {
        val view: View = LayoutInflater.from(parent.context).inflate(R.layout.list_item_video, parent, false)
        return VideoFileViewHolder(view)
    }

    override fun getItemCount(): Int {
        return data?.size ?: 0
    }

    override fun onBindViewHolder(holder: VideoFileViewHolder, position: Int) {
        if(data == null || position >= data!!.size)
        {
            return
        }

        holder.tvVideoName.text = data!![position].name

        holder.itemView.tag = position

        holder.itemView.setOnClickListener(internalItemClickerListener)
    }
}

class VideoFileViewHolder(view: View): RecyclerView.ViewHolder(view)
{
    var tvVideoName: TextView
    init
    {
        tvVideoName = itemView.findViewById(R.id.tv_name)
    }
}