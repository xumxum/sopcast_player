#!/usr/bin/perl
#tcpserver.pl

use IO::Socket::INET;

# flush after every write
$| = 1;

my ($socket,$client_socket);
my ($peeraddress,$peerport);


# creating object interface of IO::Socket::INET modules which internally does
# socket creation, binding and listening at the specified port address.
$socket = new IO::Socket::INET (
LocalHost => '',
LocalPort => '12000',
Proto => 'tcp',
Listen => 1,
Reuse => 1
) or die "ERROR in Socket Creation : $!\n";

print "SERVER Waiting for client connection on port 12000\n";

# waiting for new client connection.
$client_socket = $socket->accept();

# get the host and port number of newly connected client.
$peer_address = $client_socket->peerhost();
$peer_port = $client_socket->peerport();

print "Accepted New Client Connection From : $peeraddress, $peerport\n" ;

#Initialize
#$data = <$client_socket>;
$client_socket->recv($data,15);
print "Request: $data \n";

#send Sesion Start Confirm
$reply = "40 0 1 2\nline2\n";
print $client_socket "$reply";


print "Sleep 3\n";
sleep 10;

$socket->close();
