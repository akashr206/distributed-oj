const express = require('express');
const { MongoClient, ObjectId } = require('mongodb');
const { createClient } = require('redis');
const path = require('path');
require('dotenv').config({ path: path.resolve(__dirname, '../.env') });

function createApp(options = {}) {
  const app = express();
  app.use(express.json());

  const mongoUri = options.mongoUri || process.env.MONGO_URI || 'mongodb://127.0.0.1:27017';
  const dbName = options.dbName || process.env.MONGO_DB_NAME || 'dist_oj';
  const redisUrl = options.redisUrl || process.env.REDIS_URL || 'redis://127.0.0.1:6379';
  const queueName = options.queueName || process.env.QUEUE_NAME || 'submissions_queue';

  let mongoClient;
  let mongoDb;
  let redisClient;

  async function connectServices() {
    if (!mongoClient) {
      mongoClient = new MongoClient(mongoUri);
      await mongoClient.connect();
      mongoDb = mongoClient.db(dbName);
    }

    if (!redisClient) {
      redisClient = createClient({ url: redisUrl });
      redisClient.on('error', (error) => {
        console.error('[redis]', error.message);
      });
      await redisClient.connect();
    }
  }

  app.get('/api/health', (_req, res) => {
    res.json({ status: 'ok', service: 'distributed-oj-worker-server' });
  });

  app.post('/api/submit', async (req, res) => {
    const job = req.body;
    const allowedLanguages = ['cpp'];

    if (!job || typeof job !== 'object') {
      return res.status(400).json({ error: 'Request body must be an object.' });
    }

    const requiredFields = ['code', 'input', 'expected', 'language'];
    const missing = requiredFields.filter((field) => typeof job[field] !== 'string' || job[field].trim() === '');

    if (missing.length > 0) {
      return res.status(400).json({ error: `Missing required fields: ${missing.join(', ')}` });
    }

    const timeLimitValue = Number(job.timeLimit ?? 2000);
    const memoryLimitValue = Number(job.memoryLimit ?? 256);
    const normalizedLanguage = job.language.trim().toLowerCase();

    if (!Number.isInteger(timeLimitValue) || timeLimitValue < 100 || timeLimitValue > 8000) {
      return res.status(400).json({ error: 'timeLimit must be an integer between 100 and 8000 ms.' });
    }

    if (!Number.isInteger(memoryLimitValue) || memoryLimitValue < 32 || memoryLimitValue > 512) {
      return res.status(400).json({ error: 'memoryLimit must be an integer between 32 and 512 MB.' });
    }

    if (!allowedLanguages.includes(normalizedLanguage)) {
      return res.status(400).json({ error: `language must be one of: ${allowedLanguages.join(', ')}` });
    }

    if (typeof job.expected !== 'string' || job.expected.trim() === '') {
      return res.status(400).json({ error: 'expected output cannot be empty.' });
    }

    try {
      await connectServices();
      const curDate = Date.now();
      const submission = {
        code: job.code,
        input: job.input || '',
        expected: job.expected || '',
        language: normalizedLanguage,
        timeLimit: timeLimitValue,
        memoryLimit: memoryLimitValue,
        status: 'pending',
        output: '',
        verdict: '',
        timeTaken: null,
        memoryUsed: null,
        error: '',
        internalError: '',
        createdAt: curDate,
        updatedAt: curDate
      };

      const result = await mongoDb.collection('jobs').insertOne(submission);
      const jobId = result.insertedId.toString();
      await redisClient.rPush(queueName, jobId);

      return res.status(202).json({
        message: 'Submission accepted',
        submissionId: jobId,
        // submission: { ...submission, _id: jobId }
      });
    } catch (error) {
      console.error('[submit]', error);
      return res.status(503).json({ error: 'Worker services are unavailable', details: error.message });
    }
  });

  app.get('/api/submissions/:id', async (req, res) => {
    try {
      await connectServices();
      const job = await mongoDb.collection('jobs').findOne(
        { _id: new ObjectId(req.params.id) },
        {
          projection: {
            code: 0,
            updatedAt: 0,
            createdAt: 0,
            internalError: 0
          }
        }
      );

      if (!job) {
        return res.status(404).json({ error: 'Job not found' });
      }

      return res.json(job);
    } catch (error) {
      console.error('[jobs]', error);
      return res.status(500).json({ error: 'Unable to fetch job', details: error.message });
    }
  });

  return app;
}

function startServer(options = {}) {
  const app = createApp(options);
  const port = process.env.PORT || 3000;

  return app.listen(port, () => {
    console.log(`Worker server listening on port ${port}`);
  });
}

if (require.main === module) {
  startServer();
}

module.exports = { createApp, startServer };
