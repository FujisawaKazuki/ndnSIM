#!/usr/bin/ruby
# -*- coding: utf-8 -*-


require "pathname"




def make_hist(timeslot, count)
  return { 
    timeslot: timeslot,
    packets:  count, 
    size:     calc_kbps(count), 
  }
end


def output(file_path, hists, dhists)
  file_path.open("w") do |file|
    file.puts ["timeslot", "packets", "size", "drop-packets", "drop-size"].join("\t")
    hists.zip(dhists).each do |hist, dhist|
      file.puts [
                 hist[:timeslot],
                 hist[:packets],  hist[:size], 
                 dhist[:packets], dhist[:size], 
                ].map(&:to_s).join("\t")
    end
  end
end


def in_time?(time, first, last)
  return first <= time && time < last
end


def calc_kbps(count)
  return count * 40 * 8 / 1000.0
end



#############################################################################
#    main

if ARGV.length != 1
  $stderr.puts "USAGE: #{__FILE__} logfile"
end
log_file_path = Pathname.new(ARGV.first)
raise "LOG FILE NOT FOUND" if ! log_file_path.exist?



# read log
logs = log_file_path.open.lines.map{ |line|
  items = line.split
  {
    command:   items[0], 
    timestamp: items[1].to_f, 
    node:      items[2], 
    content:   items[3], 
  }
}


#init
end_time  = 5
interval  = 0.05
# タイムスロット区間を生成する 例: [[0.0, 0.05], [0.05, 0.1] ...]
divisions = Array.new((end_time / interval).to_i){ |i| [i * interval, (i + 1) * interval] } 
output_dir = Pathname.new("log_#{log_file_path.basename(".*")}")
output_dir.mkpath

# parse
logs.map{ |log| log[:node] }.uniq.sort.each do |node|

  # init
  send_hists  = []
  retry_hists = []
  sdrop_hists = []
  rdrop_hists = []

  # count
   divisions.each do |first, last|
    in_time_logs = logs.select{ |log| log[:node] == node && in_time?(log[:timestamp], first, last) } 
    send_hists  << make_hist(last, in_time_logs.count{ |log| log[:command] == "First"  })
    retry_hists << make_hist(last, in_time_logs.count{ |log| log[:command] == "Retry" })
    sdrop_hists << make_hist(last, in_time_logs.count{ |log| log[:command] == "First_Drop" })
    rdrop_hists << make_hist(last, in_time_logs.count{ |log| log[:command] == "Retry_Drop" })
  end

  # output
  output(output_dir + Pathname.new("node-#{node}_send.log"),  send_hists,  sdrop_hists)
  output(output_dir + Pathname.new("node-#{node}_retry.log"), retry_hists, rdrop_hists)
end


