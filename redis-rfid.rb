
require 'rubygems'
require 'sinatra'
require 'redis'
require 'uri'
require 'haml'

configure do
    uri = URI.parse("redis://localhost:6379/")
    REDIS = Redis.new(:host => uri.host, :port => uri.port, :password => uri.password)
end

get '/' do
    @temperature = %x(/opt/vc/bin/vcgencmd measure_temp)
    @rfidcard = REDIS.get('rfidcard')
    haml :index
end

get '/pi/get/temp' do
    @temperature = %x(/opt/vc/bin/vcgencmd measure_temp)
end

get '/redis/get/:myparam' do # will be used by the ajax call to refresh last rfidcard read on web page
    @rfidcard = REDIS.get(params[:myparam])
end

post '/sendout' do
    @rfidcard = params["inputRFIDcard"]
    @hours = params["inputHours"]
    # do something with these...
end

__END__
@@layout
!!!
%html
  %head
    %title redis RFID

  %body
    = yield

@@index
%h1 redis-RFID home
%form{:action => "/sendout", :method => "post"}
  %legend
    Time tracker
  %p
    %label{:for => ""} Operator
    %input{:type => "textbox", :name => "inputRFIDcard", :id => "inputRFIDcard", :value => @rfidcard }
  %p
    %label{:for => ""} Hours
    %input{:type => "textbox", :name => "inputHours", :id => "inputHours", :value => 1 }
  %p
    %input{:type => "submit", :value => "Submit!"}

%h2 Raspberry Pi extra stuff
%p= @temperature
