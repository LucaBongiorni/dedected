require 'msf/core'


class Metasploit3 < Msf::Auxiliary

        include Msf::Exploit::COA
	
	def initialize
                super(
                        'Name'          => 'DECT Call Scanner',
                        'Version'       => '$revision$',
                        'Description'   => %q{
                                This module scans for active DECT 
calls, it does not do recording yet.
                        },
                        'Author'        =>
                                ['DK <privilegedmode@gmail.com>'],
                        'References'    =>
                                [
                                        ['Dedected', 'http://www.dedected.org'],
                                ],
                        'License'       => MSF_LICENSE
                )

                register_options(
                        [

				OptString.new('VERBOSE',[false,'Be 
verbose.',true])
                        ],
                        self.class
                )

        end

	:calls

	def print_results
			print("Time\t\t\t\tRFPI\t\tChannel\n")
		@calls.each do |rfpi, data|
			print("#{data['time']}\t#{data['rfpi']}\t#{data['channel']}\t\n")

		end	
	end


	#TODO
	#def record_call(data)
	#	print_status("Synchronizing..")
	#	pp_scan_mode(data['rfpi_raw'])
	#	while(true)
	#		#data = poll
	#		#puts data
	#	end
		
	#end

	def run
		@calls = {}
		scanning = true
		#record = true
	
		trap("INT") {
			scanning = false
			stop
			close_coa
			print_status("Call scan stopped.")
			print_results
		}		

		print_status("Opening interface: #{datastore['INTERFACE']}")
		open_coa
		print_status("Using band: #{band}")
		print_status("Changing to call scan mode.")
		call_scan_mode
		print_status("Scanning for active calls..")

		while (scanning)
			data = poll
			if (data != nil)
				parsed_data = parse_call(data)
				parsed_data['time'] = Time.new
				print_status("Found active call on: #{parsed_data['rfpi']}")
				@calls[parsed_data['time']] = parsed_data
				#if (record)
				#	record_call(parsed_data)
				#end


			end

			next_channel

			if (datastore['VERBOSE'])
				print_status("Switching to channel: #{channel}")
			end
			sleep(1)
		end
	end
end
