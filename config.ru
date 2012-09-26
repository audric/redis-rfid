require 'sinatra'

set :env,  :production
disable :run

require './redis-rfid.rb'

run Sinatra::Application

