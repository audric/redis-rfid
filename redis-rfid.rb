
require 'rubygems'
require 'sinatra'
require 'redis'
require 'uri'
require 'haml'

configure do
  uri = URI.parse("redis://localhost:6379/")
  set :haml, {:format => :html5}
  REDIS = Redis.new(:host => uri.host, :port => uri.port, :password => uri.password)
end

get '/' do
  @temperature = %x(/opt/vc/bin/vcgencmd measure_temp)
#  @rfidcard = REDIS.get('rfidcard')
  haml :index
end

get '/pi/get/temp' do
  @temperature = %x(/opt/vc/bin/vcgencmd measure_temp)
end

get '/redis/get/:myparam' do # will be used by the ajax call to refresh last rfidcard read on web page
  REDIS.get(params[:myparam])
end

post '/sendout' do
  @rfidcard = params["inputRFIDcard"]
  @hours = params["inputHours"]
  # do something with these...
end
