require 'msf/core'


class Metasploit3 < Msf::Auxiliary

        include Msf::Exploit::COA
	
	def initialize
                super(
                        'Name'          => 'DECT Base Station Scanner',
                        'Version'       => '$revision$',
                        'Description'   => %q{
                                This module scans for DECT device base stations.
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
				OptString.new('VERBOSE',[false,'Be verbose.',true])
                        ],
                        self.class
                )

        end

	:base_stations

	def print_results
			print("RFPI\t\tChannel\n")
		@base_stations.each do |rfpi, data|
			print("#{data['rfpi']}\t #{data['channel']}\t\n")

		end	
	end

	def run
		@base_stations = {}
		scanning = true
		
	
		trap("INT") {
			scanning = false
			stop
			close_coa
			print_status("fp scan stopped.")
			print_results
		}		

		print_status("Opening interface: #{datastore['INTERFACE']}")
		open_coa
		print_status("Using band: #{band}")
		print_status("Changing to fp scan mode.")
		fp_scan_mode
		print_status("Scanning..")

		while (scanning)
			data = poll

			if (data != nil)
				parsed_data = parse_station(data)
				if (!@base_stations.key?(parsed_data['rfpi']))
					print_status("Found New RFPI: #{parsed_data['rfpi']}")
					@base_stations[parsed_data['rfpi']] = parsed_data
				end


			end

			next_channel

			if (datastore['VERBOSE'])
				print_status("Switching to channel: #{channel}")
			end
			sleep(1)
		end
	end
end
