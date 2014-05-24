use File::Spec;
use Test::Nginx::Socket;

# setup testing environment
my $HtmlDir    = html_dir();
my $FixtureDir = File::Spec->catfile($HtmlDir, '..', '..', 'fixtures');
my $LogDir     = File::Spec->catfile($HtmlDir, '..', 'logs');

$ENV{TEST_NGINX_FIXTURE_DIR} = $FixtureDir;
$ENV{TEST_NGINX_LOG_DIR}     = $LogDir;

# proceed with testing
repeat_each(1);
plan tests => repeat_each() * 2;

no_root_location();
run_tests();

__DATA__

=== TEST 1: log undetected device
--- config
    device_detection_miss_log  $TEST_NGINX_LOG_DIR/miss.log;
    device_detection_yaml      $TEST_NGINX_FIXTURE_DIR/devices.yml;

    location /miss_log {
        rewrite_by_lua "ngx.req.set_header('User-Agent', 'i am the night')";

        content_by_lua "
            local type = ngx.var['device_detection_type']
            local log  = '$TEST_NGINX_LOG_DIR/miss.log'

            for line in io.lines(log) do
                ngx.print(line)
            end
        ";
    }
--- request
    GET /miss_log
--- response_code: 200
--- response_body: i am the night
