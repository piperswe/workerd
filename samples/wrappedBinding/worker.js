export default {
  async fetch(req, env) {
    // env.hn is our HN api binding. Its own inner binding (internet) is not accessible.
    const reply = await env.hn.query({ tags: ["front_page"] });
    return new Response(JSON.stringify(reply));
  }
};
