#!/usr/bin/ruby -w

require 'rexml/document'  # xml parsing
include REXML
require 'open-uri'  # params escaping
require 'net/http'  # get requests
require 'json'  # tema answer parsing

OUTPUT = "./KWARC.txt"
RUN = "default"

$output = File.new(OUTPUT, "w");

$results = {}
def register_result num, resp, hasFormula
    $results[num] ||= {}
    resp_json = JSON.parse resp
    hits = resp_json["hits"]
    return if hits.nil?

    hits.each do |hit|
        fileName = hit["id"]
        score = hit["score"]
        currScore = $results[num][fileName]
        if currScore
            $results[num][fileName] = [currScore, score].max
        else
            $results[num][fileName] = score
        end
    end
end

class Hit
    attr_accessor :fileName, :score

    def initialize fileName, score
        @fileName, @score = fileName, score
    end
end

def process_results num
    ret = []
    $results[num].each { |k, v| ret << Hit.new(k,v) }
    puts "Processed results for #{num}"
    $stdout.flush
    ret.sort! {|h1, h2| h2.score <=> h1.score }
    ret[0...1000]
end


QUERY_LOCATION = "./queries.xml"
TEMA_URL = "http://localhost:8901/"
PARAM_FROM = 0
PARAM_SIZE = 100

queries_xml = File.new(QUERY_LOCATION)
queries_doc = Document.new(queries_xml)

topics = XPath.match(queries_doc, "/topics/*")
topics.each do |topic|
    num = XPath.match(topic, "./num[1]").first.text
    formulas = XPath.match(topic, "./query/formula/m:math/m:semantics/m:apply").map(&:to_s)
    if formulas.empty?
        formulas = XPath.match(topic, "./query/formula/m:math/m:semantics/m:cerror").map(&:to_s)
    end

    tema_url = URI.parse TEMA_URL

    formulas.each do |fmla|
            resp = Net::HTTP.post_form(tema_url, {
                'text' => "",
                'math' => "#{fmla}",
                'from' => "#{PARAM_FROM}",
                'size' => "#{PARAM_SIZE}"
            });

            register_result num, resp.body, (fmla != "")
            puts "Registered result #{num}"
            $stdout.flush
    end
    
end

$results.keys.each do |num|
    hits = process_results num

    next if hits.nil?
    hits.each_with_index do |h, rank|
        $output.puts "#{num}\t1\t#{h.fileName}\t#{rank + 1}\t#{h.score}\tKWARC_#{RUN}"
    end
end
$output.close
