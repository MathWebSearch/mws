#!/usr/bin/ruby -w

require 'rexml/document'  # xml parsing
include REXML
require 'open-uri'  # params escaping
require 'net/http'  # get requests
require 'json'  # tema answer parsing

OUTPUT = "./KWARC.txt"
RANDOM_HITS = "./random_hits.txt"
NEEDED_HITS = 1000
RUN = "default"

$output = File.new(OUTPUT, "w");
$random_fnames = File.new(RANDOM_HITS).lines.to_a;

$results = {}
def register_result num, resp, hasFormula
    $results[num] ||= {}
    resp_json = JSON.parse resp
    return if resp_json["hits"].class != Hash
    hits = resp_json["hits"]["hits"]
    return if hits.nil?

    hits.each do |hit|
        fileName = hit["_id"]
        score = hit["_score"]
        if hasFormula
            score += 5 # boost scores for results with formulae
        end
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
    
    $random_fnames.shuffle!
    i = -1;
    return if ret.length == 0
    while(ret.length < NEEDED_HITS)
        i += 1
        next if $results[num][$random_fnames[i]]
        $results[num][$random_fnames[i]] = "SET"
        ret << Hit.new($random_fnames[i].chomp, 0)
    end
    puts "Processed results for #{num}"
    $stdout.flush
    ret.sort! {|h1, h2| h2.score <=> h1.score }
    ret[0...1000]
end


QUERY_LOCATION = "./queries.xml"
TEMA_URL = "http://localhost:8889/"
PARAM_FROM = 0
PARAM_SIZE = 100

queries_xml = File.new(QUERY_LOCATION)
queries_doc = Document.new(queries_xml)

topics = XPath.match(queries_doc, "/topics/*")
topics.each do |topic|
    num = XPath.match(topic, "./num[1]").first.text
    keywords_str = XPath.match(topic, "./query/keyword").map(&:text).join(" ")
    formulas = XPath.match(topic, "./query/formula/m:math/m:semantics/m:apply").map(&:to_s)
    if formulas.empty?
        formulas = XPath.match(topic, "./query/formula/m:math/m:semantics/m:cerror").map(&:to_s)
    end
    formulas.push('')

    tema_url = URI.parse TEMA_URL

    formulas.each do |fmla|
            resp = Net::HTTP.post_form(tema_url, {
                'text' => "#{keywords_str}",
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
