
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

__END__
@@layout
!!!
%html
  %head
    %title Raspberry Pi redis RFID
    %link(rel="stylesheet" href="css/bootstrap.min.css")

  %body(style="background-color: white;")
    = yield
    %script(src="http://code.jquery.com/jquery-latest.js")
    %script(src="js/bootstrap.min.js")
    :javascript
      $(mywatch);
      function mywatch() {
       var xmlhttp;
       xmlhttp=new XMLHttpRequest();
       xmlhttp.open("GET","http://10.16.0.149:4567/redis/get/rfidcard",true);
       xmlhttp.send();
       xmlhttp.onreadystatechange=function()
       {
         if (xmlhttp.readyState==4 && xmlhttp.status==200)
         {
          document.getElementById("inputRFIDcard").value=xmlhttp.responseText;
         }
       }
       var mydate = new Date();
       var dd   = mydate.getDate();
       var mm   = mydate.getMonth()+1;
       var yyyy = mydate.getFullYear();
       if (dd < 10) { dd = 10 + dd };
       if (mm < 10) { mm = 10 + mm };
       document.getElementById("inputDate").value=( yyyy + "-" + mm + "-" + dd );
       setTimeout(mywatch,500);
      }

@@index
.page-header
  %h1 Raspberry Pi redis RFID home
.container
  .form-horizontal
    %form{:action => "/sendout", :method => "post"}
      .control-group
        %label{:class => "control-label", :for => "inputDate"} Date
        .controls
          .input-prepend
            %span(class="add-on")
              %i(class="icon-calendar icon-black")
            %input{:type => "textbox", :name => "inputDate", :id => "inputDate", :value => '2012-01-01' }
      .control-group
        %label{:class => "control-label", :for => "inputRFIDcard"} Operator
        .controls
          .input-prepend
            %span(class="add-on")
              %i(class="icon-user icon-black")
            %input{:type => "textbox", :name => "inputRFIDcard", :id => "inputRFIDcard", :value => @rfidcard }
      .control-group
        %label{:class => "control-label", :for => "inputHours"} Hours
        .controls
          .input-prepend
            %span(class="add-on")
              %i(class="icon-time icon-black")
            %input{:type => "textbox", :name => "inputHours", :id => "inputHours", :value => 1 }
            %button(class="btn btn-small")
              %i(class="icon-plus icon-black")
            %button(class="btn btn-small")
              %i(class="icon-minus icon-black")
      .control-group
        .controls
          %button{:type => "submit", :class => "btn btn-large btn-primary"}
            Salva!
            %i(class="icon-ok icon-white")
%h2 Raspberry Pi extra stuff
%p= @temperature
