#!/usr/bin/env python3
"""简单的并发压测脚本"""

import http.client
import time
import threading
import statistics
from concurrent.futures import ThreadPoolExecutor, as_completed

# 配置
HOST = "localhost"
PORT = 8084
TOTAL_REQUESTS = 10000
CONCURRENCY = 100

# 结果存储
results = {
    "success": 0,
    "failed": 0,
    "latencies": []
}
results_lock = threading.Lock()

def make_request():
    """发送单个请求"""
    try:
        start_time = time.time()
        
        conn = http.client.HTTPConnection(HOST, PORT, timeout=5)
        conn.request("GET", "/health")
        response = conn.getresponse()
        response.read()
        conn.close()
        
        latency = (time.time() - start_time) * 1000  # 转换为毫秒
        
        with results_lock:
            if response.status == 200:
                results["success"] += 1
            else:
                results["failed"] += 1
            results["latencies"].append(latency)
        
        return True
    except Exception as e:
        with results_lock:
            results["failed"] += 1
        return False

def main():
    print("=" * 60)
    print("  Simple Benchmark - ChatGPT Server")
    print("=" * 60)
    print(f"\nConfiguration:")
    print(f"  Target: http://{HOST}:{PORT}/health")
    print(f"  Total Requests: {TOTAL_REQUESTS}")
    print(f"  Concurrency: {CONCURRENCY}")
    print()
    
    # 热身
    print("Warming up...")
    for _ in range(10):
        make_request()
    
    with results_lock:
        results["success"] = 0
        results["failed"] = 0
        results["latencies"] = []
    
    # 开始压测
    print(f"\nStarting benchmark...")
    start_time = time.time()
    
    with ThreadPoolExecutor(max_workers=CONCURRENCY) as executor:
        futures = [executor.submit(make_request) for _ in range(TOTAL_REQUESTS)]
        
        # 显示进度
        completed = 0
        for future in as_completed(futures):
            completed += 1
            if completed % 1000 == 0:
                print(f"  Progress: {completed}/{TOTAL_REQUESTS}")
    
    total_time = time.time() - start_time
    
    # 计算统计信息
    print("\n" + "=" * 60)
    print("  Results")
    print("=" * 60)
    print(f"\nRequests:")
    print(f"  Total: {TOTAL_REQUESTS}")
    print(f"  Successful: {results['success']}")
    print(f"  Failed: {results['failed']}")
    print(f"  Success Rate: {results['success'] / TOTAL_REQUESTS * 100:.2f}%")
    
    print(f"\nPerformance:")
    print(f"  Total Time: {total_time:.2f}s")
    print(f"  Requests/sec: {TOTAL_REQUESTS / total_time:.2f}")
    
    if results["latencies"]:
        latencies = sorted(results["latencies"])
        print(f"\nLatency (ms):")
        print(f"  Min: {min(latencies):.2f}")
        print(f"  Max: {max(latencies):.2f}")
        print(f"  Mean: {statistics.mean(latencies):.2f}")
        print(f"  Median: {statistics.median(latencies):.2f}")
        print(f"  P50: {latencies[int(len(latencies) * 0.50)]:.2f}")
        print(f"  P90: {latencies[int(len(latencies) * 0.90)]:.2f}")
        print(f"  P95: {latencies[int(len(latencies) * 0.95)]:.2f}")
        print(f"  P99: {latencies[int(len(latencies) * 0.99)]:.2f}")
    
    print("\n" + "=" * 60)
    print("  Benchmark Complete")
    print("=" * 60)

if __name__ == "__main__":
    main()
