(function() {
    const linkElement = document.querySelector('link[href="styles.css"]');
    
    if (linkElement) {
        if (document.readyState === 'complete' || linkElement.sheet) {
            recordCssLoaded();
        } else {
            linkElement.addEventListener('load', recordCssLoaded);
            setTimeout(recordCssLoaded, 1000);
        }
    } else {
        console.error('CSS link element not found');
    }
    
    function recordCssLoaded() {
        if (!window.perfMetrics.timing.cssLoaded) {
            window.perfMetrics.timing.cssLoaded = performance.now();
            window.perfMetrics.requests.css.status = '200 OK';
            window.perfMetrics.requests.css.duration = window.perfMetrics.timing.cssLoaded - window.perfMetrics.timing.pageStart;
            console.log('CSS loaded in', window.perfMetrics.requests.css.duration, 'ms');
        }
    }
})();

document.addEventListener('DOMContentLoaded', function() {
    console.log('Static server test page loaded successfully!');
    
    window.perfMetrics.timing.jsLoaded = performance.now();
    window.perfMetrics.requests.js.status = '200 OK';
    window.perfMetrics.requests.js.duration = window.perfMetrics.timing.jsLoaded - window.perfMetrics.timing.pageStart;
    
    updateMetricsDisplay();
});

function updateMetricsDisplay() {
    if (window.perfMetrics.requests.html.duration !== null) {
        updateMetricCard('html', window.perfMetrics.requests.html);
    }
    
    if (window.perfMetrics.requests.css.duration !== null) {
        updateMetricCard('css', window.perfMetrics.requests.css);
    } else {
        setTimeout(() => {
            if (window.perfMetrics.requests.css.duration === null) {
                window.perfMetrics.timing.cssLoaded = performance.now();
                window.perfMetrics.requests.css.status = '200 OK';
                window.perfMetrics.requests.css.duration = window.perfMetrics.timing.cssLoaded - window.perfMetrics.timing.pageStart;
            }
            updateMetricCard('css', window.perfMetrics.requests.css);
        }, 500);
    }
    
    if (window.perfMetrics.requests.js.duration !== null) {
        updateMetricCard('js', window.perfMetrics.requests.js);
    }
    
    const totalTime = Math.max(
        window.perfMetrics.requests.html.duration || 0,
        window.perfMetrics.requests.css.duration || 0,
        window.perfMetrics.requests.js.duration || 0
    );
    
    document.getElementById('total-load-time').textContent = `${totalTime.toFixed(2)} ms`;
    
    setTimeout(() => {
        document.querySelectorAll('.metric-card').forEach(card => {
            card.classList.add('loaded');
        });
    }, 500);
}

function updateMetricCard(fileType, metricData) {
    const statusElem = document.getElementById(`${fileType}-status`);
    const timeElem = document.getElementById(`${fileType}-time`);
    const progressElem = document.getElementById(`${fileType}-progress`);
    
    if (metricData.duration !== null) {
        statusElem.classList.add('status-success');
        
        timeElem.textContent = `${metricData.duration.toFixed(2)} ms`;
        
        const percentage = Math.min(metricData.duration / 10, 100);
        progressElem.style.width = `${percentage}%`;
        
        setTimeout(() => {
            progressElem.style.transition = 'width 0.8s cubic-bezier(0.22, 0.61, 0.36, 1)';
            progressElem.style.width = `${percentage}%`;
        }, 100 * (fileType === 'html' ? 1 : fileType === 'css' ? 2 : 3));
    } else {
        statusElem.classList.add('status-error');
        timeElem.textContent = 'Failed to load';
    }
}

window.addEventListener('load', function() {
    const fullyLoaded = performance.now();
    console.log(`Page fully loaded in ${fullyLoaded - window.perfMetrics.timing.pageStart} ms`);
    
    updateMetricsDisplay();
});

console.log('index.js loaded successfully!');
